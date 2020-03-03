# study extension

extensionの勉強。写経

This product includes PHP, freely available from http://www.php.net/

## INSTALL

php-srcディレクトリにあるものとする。

    $ cd ext/
    $ git clone https://github.com/youkidearitai/study_extension
    $ cd ../
    $ ./buildconf -f && ./configure --enable-study_extension
    $ make && make test && make install

## functions

### `study_extension_dump( mixed $value )`

`var_dump`の写経、簡略とアレンジを含む。

