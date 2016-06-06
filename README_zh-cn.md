# xhprof of php7 #

在github上或者pecl都没有找到xhprof的php7版本,所以我花了两天时间整理了一下xhprof实现的细节,并顺利将其升级到PHP7版本.刚开始实现的是php7-dev版本的,但是现在发RC版了,原来的分支结果不能使用了.而经过今天的修复，在php7-rc1版本也能正常使用了.


目录：

1. 编译安装
2. 实现细节
3. 升级结果
4. 注意事项

## 编译安装 ##

编译安装比较简单，更多详情可以查看[xhprof简介](http://yaoguais.github.io/?s=md/xhprof/intro.md)

	git clone git@github.com:Yaoguais/phpng-xhprof.git ./xhprof
	cd xhprof
	phpize
	./configure
	make clean && make && make test && sudo make install
	php diff/enable.php
	ls -l diff

	自己编译的PHP7:
	git clone git@github.com:Yaoguais/phpng-xhprof.git ./xhprof
	cd xhprof
	/path/to/php7/bin/phpize
	./configure --with-php-config=/path/to/php7/bin/php-config
	make clean && make && make test && sudo make install
	/path/to/php7/bin/php diff/enable.php
	ls -l diff

编辑php.ini文件，在最后添加xhprof的配置。

	[xhprof]
	extension = phpng_xhprof.so
	xhprof.output_dir = /tmp/xhprof
	xhprof.count_prefix =

output_dir指默认的分析结果保存目录，在php脚本中也可以临时设置一个新的。
count_prefix是根据前缀做统计计算，如果为空则统计所有的调用


## 实现细节 ##

关键字：输入输出、分析源码、PHP7下编译、如何把扩展升级到PHP7

开始是我对这个扩展还是有一定认识的，平时在项目中会经常使用到，但是只是停留在使用的阶段。

因为xhprof自带了网页数据展示，那么在升级扩展的过程中，最好不要改变输入输出接口。确切点就是扩展提供的函数调用不变。再具体一点，就是函数的参数与返回值不要发生一点变化。所以我通过分析源码、结合GDB打印内存、查看PHP帮助手册等方式确定了函数的输入输出。

[更多输入输出相关的文章](http://yaoguais.github.io/?s=md/xhprof/interface.md)

然后结合以前的
[php cli执行流程](http://yaoguais.github.io/?s=md/php/cli.md)这边文章，分析这个扩展的执行流程、数据结构、实现算法等等。再对比输入输出文件，就算对整个扩展有了比较清晰的认识。最后结合自己翻译的
[如何把扩展升级到PHP7](http://yaoguais.github.io/?s=md/php/extension-php5to7.md)和
[PHP7实现的一些细节](http://yaoguais.github.io/?s=md/php/php7-vm.md)
，对比着内核代码与内核的部分实现，基本就完成了升级的初始版本。

[xhprof实现的一些细节](http://yaoguais.github.io/?s=md/xhprof/theory.md)


## 升级结果 ##

对比结果：单从测试文件看出，输入一样的情况下，输出是100%一致。

diff文件夹中是编写的测试文件，测试文件包含的xhprof诊断的每种类型：编译文件、载入文件或字符串、编译eval、内核函数、用户函数。

单元测试基本也已经完成,可以使用make test进行查看结果。当前使用的是官方编写的测试脚本，总共有12个测试用例。

后面计划整合PHP以前的版本，而不需要单独为php7另编译一个脚本。

## 注意事项 ##

由于重写了php的最重要的函数execute_ex,而大多数分析工具都会修改或重写execute_ex相关的函数或函数指针，所以可能会有一些冲突。在不能使用的情况下，请检查是否开启了phpdbg,xdebug等扩展.
