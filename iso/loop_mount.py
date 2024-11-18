import os
import subprocess

def loop_mount_iso(iso_path, mount_point):
    try:
        # Create mount point if it doesn't exist
        if not os.path.exists(mount_point):
            os.makedirs(mount_point)

        # Mount the ISO
        subprocess.run(["sudo", "mount", "-o", "loop", iso_path, mount_point], check=True)
        print(f"ISO mounted successfully at {mount_point}")
    except subprocess.CalledProcessError as e:
        print(f"Error mounting ISO: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def unmount_iso(mount_point):
    try:
        # Unmount the ISO
        subprocess.run(["sudo", "umount", mount_point], check=True)
        print(f"ISO unmounted successfully from {mount_point}")
    except subprocess.CalledProcessError as e:
        print(f"Error unmounting ISO: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

# Example usage
iso_file = "ubuntu18.iso"
mount_dir = "/mnt/iso"
loop_mount_iso(iso_file, mount_dir)



# Do operations here...

unmount_iso(mount_dir)
