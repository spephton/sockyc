# sockyc: a basic TCP socket client
`sockyc` is a basic TCP socket client for sending short messages.

Example usage:
```
sockyc your.server.com "Hello, World!"
```

`sockyc` supports DNS name resolution and by default will communicate on TCP port 1024.
Port can be specified otherwise with the `-p` flag. 

`sockyc -?` provides help and available flags.

## PROTOCOL

`sockyc` just sends the message input in one go, followed by an ASCII EOT character (x04).

It then closes the socket without reading a response.

## BUILDING:

### Non-portable version:
Has a slightly nicer CLI parser.
* requires glibc
* just run make
* source in main.c

### Portable version
Utilizes Justine Tunney's excellent Cosmopolitan libc to build an APE binary that runs
on Windows, macOS (including ARM64!), and Linux. Source in maincosmo.c

Follow the instructions to download cosmopolitan libc from (justine.lol/cosmopolitan)
```
mkdir cosmocc
cd cosmocc
wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
unzip cosmocc.zip
cd ..
```

Build with
```
cosmocc/bin/cosmocc -o sockycc maincosmo.c
```
