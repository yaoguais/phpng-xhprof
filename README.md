# Xhprof of PHP7

XHProf is a function-level hierarchical profiler for PHP
and has a simple HTML based navigational interface.
The raw data collection component is implemented in C (as a PHP extension).
The reporting/UI layer is all in PHP.

It is capable of reporting function-level inclusive and exclusive wall times,
memory usage, CPU times and number of calls for each function.

Additionally, it supports ability to compare two runs (hierarchical DIFF reports),
or aggregate results from multiple runs.

You can visit [http://pecl.php.net/package/xhprof](http://pecl.php.net/package/xhprof)
to find the official extension.

But never find the version of PHP7, So I tried.




# Installation

    git clone git@github.com:Yaoguais/phpng-xhprof.git ./xhprof
	cd xhprof
	phpize
	./configure
	make clean && make && make test && sudo make install



Then add the configuration into php.ini

    [xhprof]
    extension = phpng_xhprof.so
    xhprof.output_dir = /tmp/xhprof



If you compiled PHP7 from source code, then

    git clone git@github.com:Yaoguais/phpng-xhprof.git ./xhprof
    cd xhprof
    /path/to/php7/bin/phpize
    ./configure --with-php-config=/path/to/php7/bin/php-config
    make clean && make && make test && sudo make install




# Test

There are 12 test cases, the extension by 11,
and the rest can not be due to the result of the PHP core,
which is call\_user\_function family of functions.

You can use "make test" command to test the extension,
and there are also simple test examples.

    /path/to/php7/bin/php diff/enable.php
    ls -l diff




# Notice

This extension replaces the PHP core, so when you use other extensions like xdebug or phpdbg,
which might be impaired.

You should add the phpng-xhprof configuration options to the php.ini of the bottom,
and this is due to PHP 's startup sequence resulting from extension.




# Licence

[Apache License Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).

