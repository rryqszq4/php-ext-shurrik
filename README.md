shurrik
=======

Shurrik is a PHP extension, used for testing PHP kernel

## Environment

Linux localhost 2.6.32-279.el6.i686 #1 SMP Fri Jun 22 10:59:55 UTC 2012 i686 i686 i386 GNU/Linux

PHP 5.3.27

## Building

~~~ sh
$ ./configure --with-php-config=/usr/local/php/bin/php-config
$ make && make install

$ gcc shurrik_server.c -o shurrik_server
~~~

## Running  shurrik_server

~~~ sh
$ ./shurrik_server
~~~

## Example  trace opcode

Run test and will be tracing in cli
~~~ sh
$ php tests/002.php
~~~
~~~ sh
$ ./shurrik_server
==================== start ====================
tests/002.php
id  line  opcode        handler                             op1       op2       result
0   3     ZEND_ECHO     ZEND_ECHO_SPEC_CONST_HANDLER        abc
1   5     ZEND_PRINT    ZEND_PRINT_SPEC_CONST_HANDLER       abc                 ~(nil)
2   5     ZEND_FREE     ZEND_FREE_SPEC_TMP_HANDLER          ~(nil)
3   8     ZEND_RETURN   ZEND_RETURN_SPEC_CONST_HANDLER      1

==================== end ====================
~~~

Run in web cgi , this is a app test in yaf
![image](https://github.com/rryqszq4/shurrik/blob/master/EXT_yaf_1.png)



