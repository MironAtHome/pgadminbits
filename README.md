to produce split, create ZIP archive, build bytesplitter
```
cl /MT /TC bytesplitter.c
bytesplitter.exe .\pgAdmin.zip
```

to reassemble split into single .ZIP archive

```
git clone https://github.com/MironAtHome/pgadminbits.git
pushd pgAdmin##
copy /B pgAdmin##.zip_1 + /B pgAdmin##.zip_2 ... pgAdmin##.zip
```
