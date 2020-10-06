#!/usr/bin/env python3
import json
import os
import shutil
import sys
import timeit

OS_OVERRIDES = {
    'macos': { # https://github.com/tpoechtrager/osxcross
        'CC': 'x86_64-apple-darwin19-clang',
        'CXX': 'x86_64-apple-darwin19-clang++-libc++',
        'STRIP': 'x86_64-apple-darwin19-strip'
    },
    'windows': { # sudo apt install gcc-mingw-w64 g++-mingw-w64
        'CC': 'x86_64-w64-mingw32-gcc',
        'CXX': 'x86_64-w64-mingw32-g++',
        'STRIP': 'x86_64-w64-mingw32-strip'
    }
}

TARGETS = ['Linux', 'macOS', 'Windows']
SHORTLANDS = {
    'Linux': 'lin',
    'macOS': 'mac',
    'Windows': 'win'
}


class SomethingWentWrong(Exception):
    pass


def do(command):
    #  os.system(command)
    #  return
    code = os.system('{} >/dev/null 2>&1'.format(command))
    if code:
        raise SomethingWentWrong()


def build(target):
    overrides = OS_OVERRIDES.get(target, {})
    for key, value in overrides.items():
        os.environ[key] = value
    if target == 'linux':
        os.environ['WORKING_DIR'] = os.getcwd()
        # vcv-plugin-builder.sh is a script which should cd to the
        # vcv-plugin-builder-linux directory and run ./build.sh
        # You can get it at https://github.com/cschol/vcv-plugin-builder-linux
        # It's required to easily compile plugin for the old glibc (2.23)
        # to make it compatible with old Linux distributions.
        do('vcv-plugin-builder.sh')
    else:
        do('make clean')
        do('make -j{} dist'.format(os.cpu_count()))


with open('plugin.json', 'r') as fd:
    manifest = json.load(fd)

slug = manifest.get('slug')
version = manifest.get('version')

destination = '{}-{}'.format(slug, version)
os.makedirs(destination, exist_ok=True)

total_time = 0

for target in TARGETS:
    print('Building {} for {}...'.format(slug, target), end=' ')
    sys.stdout.flush()
    start = timeit.default_timer()
    try:
        build(target.lower())
    except SomethingWentWrong:
        print('Something went wrong when building {} for {}!'.format(slug, target))
        sys.exit(1)
    archive_name = '{}-{}-{}.zip'.format(
        slug,
        version,
        SHORTLANDS.get(target)
    )
    shutil.copy(
        'dist/{}'.format(archive_name),
        '{}/{}'.format(destination, archive_name)
    )
    end = timeit.default_timer()
    elapsed = end - start
    total_time += elapsed
    print('{}s'.format(round(elapsed, 1)))

print('Total time: {}s'.format(round(total_time, 1)))

print('Done! Release files are in {} folder.'.format(destination))
print('')
