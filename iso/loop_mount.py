import os
import subprocess
import sys
import argparse
import shutil

def run_command(command, check=True, cwd=None, stdout=None):
    try:
        subprocess.run(command, check=check, cwd=cwd, stdout=stdout)
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {' '.join(command)}")
        sys.exit(1)

def is_mounted(mount_point):
    result = subprocess.run(["mountpoint", "-q", mount_point])
    return result.returncode == 0

def unmount_if_mounted(mount_point):
    if is_mounted(mount_point):
        print(f"Unmounting {mount_point}...")
        run_command(["sudo", "umount", mount_point])

def main():
    parser = argparse.ArgumentParser(description="Modify ISO to embed project and pre-install packages.")
    parser.add_argument("iso", help="Path to the ISO file to modify")
    args = parser.parse_args()

    iso_path = os.path.abspath(args.iso)
    mount_point = "/mnt/iso_mount"
    working_dir = "/tmp/iso_working"
    base_squashfs_dir = "/tmp/squashfs_base"
    lang_squashfs_dir = "/tmp/squashfs_lang"
    chroot_dir = base_squashfs_dir  # For clarity
    output_iso_path = "modified.iso"

    # Ensure working directories are clean
    for dir_path in [mount_point, working_dir, base_squashfs_dir, lang_squashfs_dir]:
        if os.path.exists(dir_path):
            # Unmount if it's a mount point
            unmount_if_mounted(dir_path)
            # Now try to remove it
            shutil.rmtree(dir_path)
        os.makedirs(dir_path, exist_ok=True)

    # Mount the ISO
    print("Mounting the ISO...")
    run_command(["sudo", "mount", "-o", "loop", iso_path, mount_point])

    # Copy ISO contents to working directory
    print("Copying ISO contents to working directory...")
    run_command(["rsync", "-a", f"{mount_point}/", working_dir])

    # Unmount the ISO
    print("Unmounting the ISO...")
    run_command(["sudo", "umount", mount_point])

    # Paths to SquashFS files
    base_squashfs_file = os.path.join(working_dir, "casper", "minimal.standard.squashfs")
    lang_squashfs_file = os.path.join(working_dir, "casper", "minimal.standard.en.squashfs")

    # Ensure SquashFS files exist
    for squashfs_file in [base_squashfs_file, lang_squashfs_file]:
        if not os.path.exists(squashfs_file):
            print(f"Error: {squashfs_file} does not exist.")
            sys.exit(1)

    # Extract base SquashFS file
    print("Extracting base SquashFS file...")
    run_command(["sudo", "unsquashfs", "-d", base_squashfs_dir, base_squashfs_file])

    # Extract language SquashFS file into separate directory
    print("Extracting language SquashFS file...")
    run_command(["sudo", "unsquashfs", "-d", lang_squashfs_dir, lang_squashfs_file])

    # Merge only language-specific directories
    print("Merging language-specific files into base filesystem...")

    language_dirs = ["usr/share/locale", "usr/share/doc", "usr/share/man", "usr/share/help", "usr/share/i18n", "usr/lib/locale"]
    for dir_name in language_dirs:
        src_dir = os.path.join(lang_squashfs_dir, dir_name)
        dest_dir = os.path.join(base_squashfs_dir, dir_name)
        if os.path.exists(src_dir):
            # Ensure the destination directory exists
            if not os.path.exists(dest_dir):
                os.makedirs(dest_dir, exist_ok=True)
            run_command(["sudo", "rsync", "-a", f"{src_dir}/", dest_dir])

    # Ensure that mount points exist
    necessary_dirs = ["dev", "proc", "sys", "dev/pts"]
    for dir_name in necessary_dirs:
        dir_path = os.path.join(chroot_dir, dir_name)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)

    # Mount necessary filesystems
    run_command(["sudo", "mount", "--bind", "/dev", os.path.join(chroot_dir, "dev")])
    run_command(["sudo", "mount", "-t", "proc", "/proc", os.path.join(chroot_dir, "proc")])
    run_command(["sudo", "mount", "-t", "sysfs", "/sys", os.path.join(chroot_dir, "sys")])
    run_command(["sudo", "mount", "-t", "devpts", "devpts", os.path.join(chroot_dir, "dev", "pts")])

    # Copy DNS info
    run_command(["sudo", "cp", "/etc/resolv.conf", os.path.join(chroot_dir, "etc", "resolv.conf")])

    # Enter chroot and execute commands
    chroot_commands = """
    export HOME=/root
    export LC_ALL=C
    apt-get update
    apt-get install -y make g++ python3-venv
    mkdir -p /home/user/virtualdetect
    exit
    """
    print("Entering chroot environment...")
    run_command(["sudo", "chroot", chroot_dir, "/bin/bash", "-c", chroot_commands])

    # Copy your project files into the chroot environment
    print("Copying project files into the filesystem...")
    project_src = os.path.expanduser("~/virtualdetect")
    project_dst = os.path.join(base_squashfs_dir, "home", "user", "virtualdetect")
    run_command(["sudo", "cp", "-r", project_src, project_dst])

    # Fix permissions (replace 'user' with the actual username)
    run_command(["sudo", "chroot", chroot_dir, "chown", "-R", "user:user", "/home/user/virtualdetect"])

    # Clean up chroot environment
    print("Cleaning up chroot environment...")
    run_command(["sudo", "umount", os.path.join(chroot_dir, "dev", "pts")])
    run_command(["sudo", "umount", os.path.join(chroot_dir, "proc")])
    run_command(["sudo", "umount", os.path.join(chroot_dir, "sys")])
    run_command(["sudo", "umount", os.path.join(chroot_dir, "dev")])

    # Repack the base SquashFS file
    print("Repacking base SquashFS file...")
    os.remove(base_squashfs_file)
    run_command(["sudo", "mksquashfs", base_squashfs_dir, base_squashfs_file, "-comp", "xz"])

    # Rebuild the ISO
    print("Rebuilding the ISO...")
    os.chdir(working_dir)
    run_command([
        "sudo", "genisoimage",
        "-D", "-r", "-V", "Custom ISO",
        "-cache-inodes", "-J", "-l",
        "-b", "isolinux/isolinux.bin",
        "-c", "isolinux/boot.cat",
        "-no-emul-boot", "-boot-load-size", "4",
        "-boot-info-table",
        "-o", output_iso_path,
        "."
    ])

    print(f"Modified ISO created at {os.path.join(working_dir, output_iso_path)}")

if __name__ == "__main__":
    main()
