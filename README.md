# xhprof PHP7版 #

在github上或者pecl都没有找到xhprof的phpng版本，所以我花了两天时间整理了一下xhprof实现的细节，并顺利将其升级到PHPNG版本。目前实现的效果是扩展输入输出完全一致，包括文件路径也一模一样。由于是在原来的基本上修改的，而且PHP7内核跟以前的版本差别较大，所以该版本是只支持PHP7的。如果有错误或者不完美的地方，还请指出，谢谢。

注: 目前正从PHP7.0.0-dev升级到PHP7.0.0-RC1,当前分支暂时还不能支持所有的功能.

Going On !

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
	make clean && make && sudo make install
	php diff/enable.php
	ls -l diff

	自己编译的PHP7:
	git clone git@github.com:Yaoguais/phpng-xhprof.git ./xhprof
	cd xhprof
	/path/to/php7/bin/phpize
	./configure --with-php-config=/path/to/php7/bin/php-config
	make clean && make && sudo make install
	/path/to/php7/bin/php diff/enable.php
	ls -l diff

编辑php.ini文件，在最后添加xhprof的配置。

	[xhprof]
	extension = xhprof.so
	xhprof.output_dir = /tmp/xhprof

output_dir指默认的分析结果保存目录，在php脚本中也可以临时设置一个新的。


## 实现细节 ##

关键字：输入输出、分析源码、PHP7下编译、如何把扩展升级到PHP7

开始是我对这个扩展还是有一定认识的，平时在项目中会经常使用到，但是只是停留在使用的阶段。当我看到源代码两千多行的时候，觉得升级还是有可能的。

首先我知道，升级一个扩展，首要保证的就是扩展提供的函数调用最好不变。再具体一点，就是函数的参数与返回值不要发生一点变化。所以我通过分析源码、结合GDB打印内存、查看PHP帮助手册等方式确定了函数的输入输出。

[更多输入输出相关的文章](http://yaoguais.github.io/?s=md/xhprof/interface.md)

然后结合以前的
[php cli执行流程](http://yaoguais.github.io/?s=md/php/cli.md)这边文章，分析这个扩展的执行流程、数据结构、实现算法等等。再对比输入输出文件，就算对整个扩展有了比较清晰的认识。最后结合自己翻译的
[如何把扩展升级到PHP7](http://yaoguais.github.io/?s=md/php/extension-php5to7.md)和
[PHP7实现的一些细节](http://yaoguais.github.io/?s=md/php/php7-vm.md)
，对比着内核代码与内核的部分实现，基本就完成了升级的初始版本。

最后，再一点一点对比同样的测试文件在php5.4下面的输出，完善并修改代码。现在的版本也就完成了。

[xhprof实现的一些细节](http://yaoguais.github.io/?s=md/xhprof/theory.md)


## 升级结果 ##


## 注意事项 ##


