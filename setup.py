from distutils.core import setup, Extension

module = Extension('tgcrypto',
                   sources=['tgcryptomodule.c', 'aes.c'])

setup (name='Tg-Crypto',
       version='0.1',
       description='Cryptographic utilities for Telegram',
       ext_modules=[module])
