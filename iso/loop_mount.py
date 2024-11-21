import os
import subprocess
import sys
import argparse
import shutil


def is_mounted(mount_point):
    try:
        # Check if the mount point is already mounted
        result = subprocess.run(["mountpoint", "-q", mount_point])
        return result.returncode == 0
    except Exception as e:
        print(f"Error checking mount point: {e}")
        return False


def loop_mount_iso(iso_path, mount_point):
    try:
        # Check if the mount point is already in use
        if is_mounted(mount_point):
            print(f"Mount point {mount_point} is already in use. Attempting to unmount.")
            unmount_iso(mount_point)

        # Create mount point if it doesn't exist
        if not os.path.exists(mount_point):
            os.makedirs(mount_point)

        # Mount the ISO
        subprocess.run(["sudo", "mount", "-o", "loop", iso_path, mount_point], check=True)
        print(f"ISO mounted successfully at {mount_point}")
    except subprocess.CalledProcessError as e:
        print(f"Error mounting ISO: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)


def unmount_iso(mount_point):
    try:
        # Unmount the ISO
        subprocess.run(["sudo", "umount", mount_point], check=True)
        print(f"ISO unmounted successfully from {mount_point}")
    except subprocess.CalledProcessError as e:
        print(f"Error unmounting ISO: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        # Clean up mount directory if it's empty
        if os.path.exists(mount_point) and not os.listdir(mount_point):
            os.rmdir(mount_point)


def copy_files_from_iso(mount_point, temp_dir):
    try:
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)
        subprocess.run(["cp", "-a", f"{mount_point}/.", temp_dir], check=True)
        print(f"Copied ISO contents to {temp_dir}")
    except Exception as e:
        print(f"Error copying files from ISO: {e}")
        sys.exit(1)


def embed_tools_and_scripts(src_path, temp_dir):
    try:
        # Paths for the tools and target destination inside the ISO
        vm_detection_tool = os.path.join(src_path, "vm_detection")
        
        target_bin_dir = os.path.join(temp_dir, "usr", "local", "bin")

        # Ensure target directory exists
        os.makedirs(target_bin_dir, exist_ok=True)

        # Copy the tools into the target directory in the ISO
        shutil.copy(vm_detection_tool, target_bin_dir)
        
        print(f"Embedded vm_detection and vm_mitigation tools into {target_bin_dir}")

        # Optional: Add a startup script to ensure they run on boot
        startup_script_path = os.path.join(temp_dir, "etc", "init.d", "vm_startup")
        startup_script_content = """#!/bin/bash
/usr/local/bin/vm_detection -a

# Custom startup script for anti-virtualization tools
exit 0
"""
        # Write the startup script to init.d or create it if it does not exist
        with open(startup_script_path, "w") as startup_script:
            startup_script.write(startup_script_content)

        # Create symlink to run script on startup
        subprocess.run(["ln", "-s", startup_script_path, os.path.join(temp_dir, "etc", "rc.d", "S99vm_startup")], check=True)

        # Ensure startup script is executable
        os.chmod(startup_script_path, 0o755)
        print(f"Added startup script to {startup_script_path}")

    except Exception as e:
        print(f"Error embedding tools and startup script: {e}")
        sys.exit(1)




def main():
    # Simplified argument parsing to just require the ISO file path
    parser = argparse.ArgumentParser(description="Automatically mount, modify, and repack an ISO file.")
    parser.add_argument("iso", help="Path to the ISO file to modify")
    args = parser.parse_args()

    iso_path = args.iso
    mount_point = "/mnt/iso_mount"
    temp_dir = "/tmp/modified_iso"
    output_iso_path = "modified.iso"

    # Mount ISO
    loop_mount_iso(iso_path, mount_point)

    # Copy files from ISO to temp directory for modification
    copy_files_from_iso(mount_point, temp_dir)

    # Unmount ISO since it's not needed after copying
    unmount_iso(mount_point)

    # Embed anti-virtualization tools and scripts into the copied files
    embed_tools_and_scripts("../src", temp_dir)

    # Repack the modified files into a new ISO
    repack_iso(temp_dir, output_iso_path)


def repack_iso(temp_dir, output_iso_path):
    try:
        # Repack the modified files into a new ISO
        subprocess.run([
            "mkisofs", "-o", output_iso_path, "-b", "isolinux/isolinux.bin",
            "-c", "boot.cat", "-no-emul-boot", "-boot-load-size", "4",
            "-boot-info-table", "-J", "-R", "-V", "Modified ISO", temp_dir
        ], check=True)
        print(f"Repacked ISO created at {output_iso_path}")
    except subprocess.CalledProcessError as e:
        print(f"Error creating modified ISO: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
