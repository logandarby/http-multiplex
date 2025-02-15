#!/bin/bash

read -r -d '' VAR << EOM
GET /test.html undefined
Host: localhost:8080
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:135.0) 
Gecko/20100101 Firefox/135.0
Accept: 
text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Connection: keep-alive
EOM

while true; do
  echo "$VAR" | radamsa | nc localhost $1 -w 10
done

