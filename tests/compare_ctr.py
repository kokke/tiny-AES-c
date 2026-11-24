"""Build, verify and time the original-loop, block-loop and AES-NI CTR paths.

Run from the repository root:
    python -B tests/compare_ctr.py --output ctr-comparison.json
"""
import argparse
import ctypes as ct
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import tempfile

from cryptography import __version__ as cryptography_version
from cryptography.hazmat.backends.openssl.backend import backend
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

ROOT = Path(__file__).resolve().parents[1]
PROBE = ROOT / "tests" / "ctr_compare_probe.c"
SIZES = (64, 4096, 1048576)
TARGET_BYTES = 16 * 1024 * 1024
VARIANTS = ("original_loop", "portable_blocks", "aesni_dispatch")
IV = bytes.fromhex("f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff")
PLAIN = bytes.fromhex(
    "6bc1bee22e409f96e93d7e117393172a"
    "ae2d8a571e03ac9c9eb76fac45af8e51"
    "30c81c46a35ce411e5fbc1191a0a52ef"
    "f69f2445df4f9b17ad2b417be66c3710")
VECTORS = {
    128: ("2b7e151628aed2a6abf7158809cf4f3c",
          "874d6191b620e3261bef6864990db6ce"
          "9806f66b7970fdff8617187bb9fffdff"
          "5ae4df3edbd5d35e5b4f09020db03eab"
          "1e031dda2fbe03d1792170a0f3009cee"),
    192: ("8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
          "1abc932417521ca24f2b0459fe7e6e0b"
          "090339ec0aa6faefd5ccc2c6f4ce8e94"
          "1e36b26bd1ebc670d1bd1d665620abf7"
          "4f78a7f6d29809585a97daec58c6b050"),
    256: ("603deb1015ca71be2b73aef0857d7781"
          "1f352c073b6108d72d9810a30914dff4",
          "601ec313775789a5b7a7f504bbf3d228"
          "f443e3ca4d62b59aca84e990cacaf5c5"
          "2b0930daa23de94ce87017ba2d84988d"
          "dfc9c58db67aada613c2dd08457941a6"),
}


def run(command, cwd=None):
    subprocess.run([str(part) for part in command], cwd=cwd, check=True)


def build_variant(directory, compiler, bits, variant):
    suffix = ".dll" if os.name == "nt" else ".so"
    common = ["-std=c99", "-O3", "-DNDEBUG", "-DCBC=0", "-DECB=0",
              f"-DAES{bits}=1", "-Wall", "-Wextra", "-Werror", "-I", ROOT]
    if os.name != "nt":
        common.append("-D_POSIX_C_SOURCE=200809L")
    defines = []
    if variant == "original_loop":
        defines.append("-DTINY_AES_CTR_ORIGINAL=1")
    elif variant == "aesni_dispatch":
        defines.append("-DTINY_AES_AESNI=1")
    aes_obj = directory / f"{variant}-{bits}-aes.o"
    probe_obj = directory / f"{variant}-{bits}-probe.o"
    objects = [aes_obj, probe_obj]
    pic = ["-fPIC"] if os.name != "nt" else []
    run([compiler, *common, *defines, *pic, "-c", ROOT / "aes.c", "-o", aes_obj])
    run([compiler, *common, *defines, *pic, "-c", PROBE, "-o", probe_obj])
    if variant == "aesni_dispatch":
        aesni_obj = directory / f"{variant}-{bits}-aesni.o"
        run([compiler, *common, *defines, *pic, "-maes", "-c", ROOT / "aesni.c",
             "-o", aesni_obj])
        objects.append(aesni_obj)
    library = directory / f"{variant}-{bits}{suffix}"
    link_flags = ["-shared"]
    if os.name == "nt":
        link_flags.append("-Wl,--export-all-symbols")
    run([compiler, *link_flags, *objects, "-o", library])
    lib = ct.CDLL(str(library))
    lib.probe_backend.argtypes = []
    lib.probe_backend.restype = ct.c_int
    lib.probe_transform.argtypes = [ct.c_void_p, ct.c_void_p, ct.c_void_p,
                                    ct.c_size_t, ct.c_void_p, ct.c_size_t, ct.c_int]
    lib.probe_transform.restype = None
    lib.probe_reset.argtypes = [ct.c_void_p, ct.c_void_p, ct.c_void_p,
                                ct.c_void_p, ct.c_void_p, ct.c_size_t]
    lib.probe_reset.restype = None
    lib.probe_bench_ns.argtypes = [ct.c_void_p, ct.c_void_p, ct.c_void_p,
                                   ct.c_size_t, ct.c_size_t]
    lib.probe_bench_ns.restype = ct.c_uint64
    return lib


def reference(key, iv, data):
    transform = Cipher(algorithms.AES(key), modes.CTR(iv)).encryptor()
    return transform.update(data) + transform.finalize()


def transform(lib, key, iv, data, chunks, force_portable=False):
    if sum(chunks) != len(data):
        raise ValueError("chunk schedule does not cover the input")
    guarded = ct.create_string_buffer(b"\xa5" + data + b"\x5a")
    schedule = (ct.c_size_t * len(chunks))(*chunks)
    pointer = ct.byref(guarded, 1) if data else None
    lib.probe_transform(key, iv, pointer, len(data), schedule, len(chunks), force_portable)
    if guarded.raw[0] != 0xA5 or guarded.raw[len(data) + 1] != 0x5A:
        raise AssertionError("write outside the in-place buffer")
    return guarded.raw[1:1 + len(data)]


def schedules(length, rng):
    result = [[length], [0, length, 0]]
    for cut in (1, 15, 16, 17, 31):
        if cut <= length:
            result.append([cut, length - cut])
    if length <= 257:
        result.append([1] * length if length else [0])
    else:
        chunks, remaining = [], length
        while remaining:
            take = min(remaining, rng.randint(1, 4097))
            chunks.extend((0, take))
            remaining -= take
        result.append(chunks)
    return result


def verify(lib, bits, variant):
    failures = []
    cases = 0
    key_hex, cipher_hex = VECTORS[bits]
    key, cipher = bytes.fromhex(key_hex), bytes.fromhex(cipher_hex)
    for cut in range(len(PLAIN) + 1):
        actual = transform(lib, key, IV, cipher, [cut, len(PLAIN) - cut])
        cases += 1
        if actual != PLAIN:
            failures.append({"source": "NIST SP 800-38A", "length": len(PLAIN),
                             "chunks": [cut, len(PLAIN) - cut]})

    rng = random.Random(0xC7A000 + bits)
    for length in (0, 1, 15, 16, 17, 31, 32, 33, 255, 256, 257, 1048576):
        generated_key = rng.randbytes(bits // 8)
        iv = rng.randbytes(14) + b"\xff\xff" if length == 257 else rng.randbytes(16)
        plain = rng.randbytes(length)
        cipher = reference(generated_key, iv, plain)
        for chunks in schedules(length, rng):
            actual = transform(lib, generated_key, iv, cipher, chunks)
            cases += 1
            if actual != plain:
                mismatch = next((i for i, pair in enumerate(zip(actual, plain))
                                 if pair[0] != pair[1]), min(len(actual), len(plain)))
                failures.append({"source": "generated", "length": length,
                                 "chunks": chunks[:20], "first_mismatch": mismatch})

    reset_plain = rng.randbytes(33)
    first_iv, second_iv = rng.randbytes(16), rng.randbytes(16)
    first = ct.create_string_buffer(reference(key, first_iv, b"x"))
    second = ct.create_string_buffer(reference(key, second_iv, reset_plain))
    lib.probe_reset(key, first_iv, second_iv, first, second, len(reset_plain))
    cases += 1
    if second.raw[:len(reset_plain)] != reset_plain:
        failures.append({"source": "set_iv reset", "length": len(reset_plain)})

    return {"variant": variant, "key_bits": bits, "cases": cases,
            "wrong_plaintexts": len(failures), "failures": failures[:3],
            "aesni_selected": bool(lib.probe_backend())}


def benchmark(libraries):
    rng = random.Random(0xB3AC7)
    key, iv = rng.randbytes(16), rng.randbytes(16)
    results = []
    for length in SIZES:
        seed = rng.randbytes(length)
        iterations = max(1, TARGET_BYTES // length)
        values = {name: [] for name in VARIANTS}
        for sample in range(11):
            order = VARIANTS[sample % len(VARIANTS):] + VARIANTS[:sample % len(VARIANTS)]
            for name in order:
                data = ct.create_string_buffer(seed)
                elapsed = libraries[name].probe_bench_ns(key, iv, data, length, iterations)
                if sample:
                    values[name].append(elapsed / iterations)
        for name in VARIANTS:
            median = statistics.median(values[name])
            results.append({"variant": name, "bytes": length,
                            "iterations_per_sample": iterations,
                            "values_ns": [round(value, 1) for value in values[name]],
                            "median_ns": round(median, 1),
                            "median_mib_s": round(length / median * 1e9 / 1048576, 1)})
    return results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cc", default=os.environ.get("CC", "gcc"))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    compiler = subprocess.check_output([args.cc, "--version"], text=True).splitlines()[0]
    cpu = subprocess.check_output(
        ["powershell", "-NoProfile", "-Command",
         "(Get-CimInstance Win32_Processor | Select-Object -First 1 -ExpandProperty Name).Trim()"],
        text=True).strip() if os.name == "nt" else platform.processor()
    report = {"compiler": compiler,
              "release_flags": "-std=c99 -O3 -DNDEBUG -DCBC=0 -DECB=0 -Wall -Wextra -Werror",
              "aesni_source_flag": "-maes (aesni.c only)", "cpu": cpu,
              "platform": platform.platform(),
              "reference": f"cryptography {cryptography_version}; {backend.openssl_version_text()}",
              "vectors": "NIST SP 800-38A F.5.2, F.5.4, F.5.6",
              "overlap": "exact in-place operation; the single-buffer API cannot express partial overlap",
              "correctness": [], "timings": []}
    handles = []
    with tempfile.TemporaryDirectory(ignore_cleanup_errors=True) as temporary:
        build = Path(temporary)
        benchmark_libraries = {}
        for bits in VECTORS:
            for variant in VARIANTS:
                lib = build_variant(build, args.cc, bits, variant)
                handles.append(lib)
                result = verify(lib, bits, variant)
                report["correctness"].append(result)
                print(f"{variant} AES-{bits}: {result['cases']} cases, "
                      f"{result['wrong_plaintexts']} wrong")
                if result["wrong_plaintexts"]:
                    raise AssertionError(result)
                if bits == 128:
                    benchmark_libraries[variant] = lib
        if not benchmark_libraries["aesni_dispatch"].probe_backend():
            raise RuntimeError("AES-NI is not available on this benchmark CPU")
        # Exercise the portable fallback in the AES-NI-enabled binary too.
        key, cipher = map(bytes.fromhex, VECTORS[128])
        if transform(benchmark_libraries["aesni_dispatch"], key, IV, cipher,
                     [1, 15, 17, 31], force_portable=True) != PLAIN:
            raise AssertionError("forced portable fallback differs")
        report["forced_portable_fallback"] = "passed"
        report["timings"] = benchmark(benchmark_libraries)
        if os.name == "nt":
            import _ctypes
            for lib in handles:
                _ctypes.FreeLibrary(lib._handle)
            handles.clear()

    for item in report["timings"]:
        print({key: value for key, value in item.items() if key != "values_ns"})
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
