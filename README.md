to produce split, create ZIP archive, build bytesplitter
cl /MTT /CP bytesplitter.c
bytesplitter pgAdmin.zip
to reassemble split into single .ZIP archive
git clone https://github.com/MironAtHome/pgadminbits.git
pushd pgAdmin##
copy /B pgAdmin##.zip_1 + /B pgAdmin##.zip_2 ... pgAdmin##.zip
