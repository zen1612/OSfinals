arr = bytearray(1024 * 1024 * 500)
for i in range(0, len(arr), 4096):
    arr[i] = 1

#/usr/bin/time -v python3 page.py

