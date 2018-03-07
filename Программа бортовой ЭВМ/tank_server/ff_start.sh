#!/bin/bash
ffserver &
ffmpeg -s 352x288 -r 30 -f video4linux2 -i /dev/video0 http://localhost:4444/feed.ffm