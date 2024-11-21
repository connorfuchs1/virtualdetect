        _        _                  _      _        _                _   
       (_)      | |                | |    | |      | |              | |  
__   __ _  _ __ | |_  _   _   __ _ | |  __| |  ___ | |_   ___   ___ | |_ 
\ \ / /| || '__|| __|| | | | / _` || | / _` | / _ \| __| / _ \ / __|| __|
 \ V / | || |   | |_ | |_| || (_| || || (_| ||  __/| |_ |  __/| (__ | |_ 
  \_/  |_||_|    \__| \__,_| \__,_||_| \__,_| \___| \__| \___| \___| \__|
                                                                        
                                                            
                                                            
If you want to start with a latest version Ubuntu / Debian VM:
--------------------------------------------------------------
Clone the repo, run "make" on the top level makefile. 
Activate the virtual environment via: "source venv/bin/activate".
Then, run "make run_flask" and navigate to your browser at http://127.0.0.1:5000.
Select your desired ISO file, and download it.

This ISO file will then be routed to a loop mount script, where known virtualization artifacts will be removed.
From the loop mount, the ISO file will then be piped into a proxmox VM instance, automatically deployed, and further vm detection and mitigations will be performed.
