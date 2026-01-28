import os
import sys
import time
import subprocess
import select

# Ensure we are in 379/Assignment2
if os.path.basename(os.getcwd()) != "Assignment2":
    if os.path.exists("379/Assignment2"):
        os.chdir("379/Assignment2")
    else:
        print("Could not find directory")
        sys.exit(1)

# Create FIFOs if they don't exist
if not os.path.exists("fifo_0_1"):
    os.mkfifo("fifo_0_1")
if not os.path.exists("fifo_1_1"):
    os.mkfifo("fifo_1_1")

print("Starting controller...")
# Start controller
proc = subprocess.Popen(["./a2sdn", "cont", "1"],
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        text=True,
                        bufsize=0) # Unbuffered

print("Opening FIFOs...")
# Open fifo_0_1 for writing
f01 = os.open("fifo_0_1", os.O_WRONLY)

# Helper to read output with timeout
def get_response(proc, expected_substr, timeout=2.0):
    start = time.time()
    found = False
    while time.time() - start < timeout:
        reads, _, _ = select.select([proc.stdout], [], [], 0.1)
        if reads:
            line = proc.stdout.readline()
            if line:
                print(f"Controller Output: {line.strip()}")
                if expected_substr in line:
                    return line.strip()
    return None

# Send first message (to satisfy the initial rcvFrame)
msg1 = "-1;-1;100-110;sw1;OPEN"
print(f"Sending msg1: {msg1}")
os.write(f01, msg1.encode())

print("Checking for response 1...")
out = get_response(proc, "recieved msg")
if out:
    print("SUCCESS: Received response for msg1")
else:
    print("WARNING: Did not receive expected response for msg1")

# Send second message
msg2 = "-1;-1;100-110;sw1;QUERY"
print(f"Sending msg2: {msg2}")
os.write(f01, msg2.encode())

print("Checking for response 2...")
out = get_response(proc, "recieved msg")

if out:
    print("SUCCESS: Received response for msg2")
else:
    print("FAILURE: Did not receive response for msg2 (Loop Unresponsive)")

proc.terminate()
try:
    proc.wait(timeout=1)
except:
    proc.kill()

os.close(f01)
