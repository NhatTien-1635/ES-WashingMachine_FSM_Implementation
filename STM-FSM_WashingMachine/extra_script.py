Import("env")
import subprocess

def make_hex(source, target, env):
    elf = str(target[0])
    hexfile = elf.rsplit(".", 1)[0] + ".hex"
    cmd = [env.subst("$OBJCOPY"), "-O", "ihex", "-R", ".eeprom", elf, hexfile]
    print("Building", hexfile)
    subprocess.check_call(cmd)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", make_hex)