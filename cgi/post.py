#!/usr/bin/env python3
# post_test.py

import os
import sys
import urllib.parse

# Only handle POST
if os.environ.get('REQUEST_METHOD', '') == 'POST':
    try:
        # Read content length
        length = int(os.environ.get('CONTENT_LENGTH', 0))
    except ValueError:
        length = 0

    # Read raw POST data from stdin
    post_data = sys.stdin.read(length)

    # Parse key-value pairs (x-www-form-urlencoded)
    params = urllib.parse.parse_qs(post_data)

    name = params.get('name', ['Guest'])[0]
    age  = params.get('age', ['unknown'])[0]

    print(f"Received via POST:\nName: {name}\nAge: {age}")

else:
    print("Content-Type: text/plain\n")
    print("Send a POST request with name and age.")