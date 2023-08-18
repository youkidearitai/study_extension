# study extension

extensionの勉強。写経

This product includes PHP, freely available from http://www.php.net/

## INSTALL

### 静的なインストール

php-srcディレクトリにあるものとする。

    $ cd ext/
    $ git clone https://github.com/youkidearitai/study_extension
    $ cd ../
    $ ./buildconf -f && ./configure --enable-study_extension
    $ make && make test && make install

### 動的なインストール

phpizeを使う。PHPの本体のインストール先を、 `--prefix=$HOME/php-master` とする。

    $ git clone https://github.com/youkidearitai/study_extension
    $ cd study_extension
    $ ~/php-master/bin/phpize
    $ ./configure --with-php-config=$HOME/php-master/bin/php-config
    $ make && make test && make install

#### php.ini

    $ vi ~/php-master/lib/php.ini
    extension_dir=/path/to/php_extension_dir/
    extension=study_extension

## functions

### `study_extension_dump( mixed $value )`

`var_dump`の写経、簡略とアレンジを含む。

### `study_extension_print_backtrace([ int $options, int $limit ])`

`debug_print_backtrace`の写経。

### `study_extension_nop()`

何もしない。

### `study_extension_phpinfo(int $flag)`

`phpinfo`を[tekitoh-memdhoi.info](https://tekitoh-memdhoi.info)のデザインっぽくしたもの。

## constants

### `STUDY_EXTENSION_CONFIGURE_COMMAND`

configureオプションを格納している定数。
