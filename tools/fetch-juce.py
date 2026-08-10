"""Extract the cached JUCE tarball into a target clone as ./JUCE.

Usage: python tools/fetch-juce.py [target-dir]

Kept in-repo because the JUCE submodule fetch stalls on this network while
the release tarball downloads in seconds. Anyone reproducing the acceptance
run needs this to work the first time.
"""
import os
import shutil
import sys
import tarfile
import urllib.request

JUCE_VERSION = "8.0.12"
URL = f"https://codeload.github.com/juce-framework/JUCE/tar.gz/refs/tags/{JUCE_VERSION}"


def main() -> int:
    target = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
    dest = os.path.join(target, "JUCE")

    if os.path.isdir(os.path.join(dest, "modules")):
        print("JUCE already present:", dest)
        return 0

    cache = os.path.join(os.environ.get("TEMP", "."), f"juce{JUCE_VERSION.replace('.', '')}.tar.gz")
    if not os.path.isfile(cache):
        print("downloading", URL, flush=True)
        urllib.request.urlretrieve(URL, cache)
    print("tarball:", cache, os.path.getsize(cache), "bytes", flush=True)

    extracted = os.path.join(target, f"JUCE-{JUCE_VERSION}")
    if os.path.isdir(extracted):
        shutil.rmtree(extracted)

    with tarfile.open(cache) as t:
        t.extractall(target)

    # The submodule leaves an empty JUCE dir behind; replace it.
    if os.path.isdir(dest):
        shutil.rmtree(dest)
    shutil.move(extracted, dest)

    ok = os.path.isdir(os.path.join(dest, "modules", "juce_core"))
    print("JUCE ready:", ok, flush=True)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
