# version 1.0

import re
import psutil
import time
import serial
import subprocess
import math
import sys

transmission_cmd = "transmission-remote -l"

def make_size_str(size):    # Creates a string for output. Input - size in bytes
    size = round(size/1073741824)   # 1073741824 = 1024^3
    if size > 999:
        size /= 1000
        ending = "TB"
        if size >= 10: size = round(size)
    else:
        ending = "GB"
    return(str(size)+ending)

def make_speed_str(speed):   # Creates a string for output. Input - speed in kilobytes/s
    if speed >= 10000:
        speed = str(round(speed/1000,0))[:-2] + "MB/s"
    elif speed < 10000 and speed >= 1000:
        speed = str(round(speed/1000,1)) + "MB/s"
    else:
        speed = str(speed)[:-2] + "KB/s"
    return speed

if len(sys.argv) != 2:
    print("Usage: status_LED <COM port>")
    sys.exit(0)

arduino = serial.Serial(sys.argv[1])

while True:
    active = 0
    try:
        tlist = subprocess.check_output(transmission_cmd, shell=True).decode('utf-8')
    except:
        continue
    lines = tlist.split('\n')

    for line in lines:
        line = re.sub(' +', ' ', line).lstrip()
        data = line.split(" ")
        if len(data) < 3: continue

        if data[0] == "ID":
            continue
        elif data[0] == "Sum:":
            speed = float(data[4])
        elif data[1] != "100%":
            active += 1
            
    free_space_storage = make_size_str(psutil.disk_usage("/storage").free)
    free_space_torrents = make_size_str(psutil.disk_usage("/torrents").free)
    
    data = "{:>2} Ts DL:{:>7}T:{:>5}  S:{:>5}".format(str(active), make_speed_str(speed), free_space_torrents, free_space_storage)
    arduino.write(data.encode('utf-8'))
    #print(data)
    time.sleep(0.5)
arduino.close()
