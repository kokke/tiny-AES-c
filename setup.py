from codecs import open
from sys import argv
import os
import re

from setuptools import find_packages, setup, Extension


def main():
    if len(argv) >= 2 and argv[1] == 'pypi':
        from subprocess import run
        from shutil import rmtree

        for x in ('build', 'dist', 'cryptg.egg-info'):
            rmtree(x, ignore_errors=True)
        run('python3 setup.py sdist', shell=True)
        run('python3 setup.py bdist_wheel', shell=True)
        run('twine upload dist/*', shell=True)
        for x in ('build', 'dist', 'cryptg.egg-info'):
            rmtree(x, ignore_errors=True)

    else:
        with open('README.rst', encoding='utf-8') as f:
            long_description = f.read()

        module = Extension('cryptg',
                           sources=['cryptgmodule.c', 'aes.c'])

        setup(
            name='cryptg',
            version='0.1',
            description="Cryptographic utilities for Telegram",
            long_description=long_description,

            url='https://github.com/Lonami/cryptg',
            download_url='https://github.com/Lonami/cryptg/releases',

            author='Lonami Exo',
            author_email='totufals@hotmail.com',

            license='CC0',

            # https://pypi.python.org/pypi?%3Aaction=list_classifiers
            classifiers=[
                'Development Status :: 3 - Alpha',

                'Intended Audience :: Developers',
                'Topic :: Security :: Cryptography',

                'License :: CC0 1.0 Universal (CC0 1.0) Public Domain Dedication',

                'Programming Language :: Python :: 3',
                'Programming Language :: Python :: 3.3',
                'Programming Language :: Python :: 3.4',
                'Programming Language :: Python :: 3.5',
                'Programming Language :: Python :: 3.6'
            ],
            keywords='telegram crypto cryptography mtproto aes',
            packages=find_packages(exclude=[]),
            install_requires=[],
            ext_modules=[module]
        )


if __name__ == '__main__':
    main()
