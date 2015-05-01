# xhprof PHP7版 #

在github上或者pecl都没有找到xhprof的phpng版本，所以我花了两天时间整理了一下xhprof实现的细节，并顺利将其升级到PHPNG版本。目前实现的效果是扩展输入输出完全一致，包括文件路径也一模一样。由于是在原来的基本上修改的，而且PHP7内核跟以前的版本差别较大，所以该版本是只支持PHP7的。如果有错误或者不完美的地方，还请指出，谢谢。

目录：

1. 编译安装
2. 实现细节
3. 升级结果

## 编译安装 ##

编译安装比较简单，更多详情可以查看[xhprof简介](http://yaoguais.github.io/?s=md/xhprof/intro.md)

	git clone git@github.com:Yaoguais/phpng-xhprof.git ./xhprof
	cd xhprof
	phpize
	./configure
	make && make install

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

(对比结果：从测试文件看出，输入一样的情况下，输出达到了100%一致。)

目前还没有做更详细的单元测试，只是编写了一个测试文件，这个测试文件包含的xhprof诊断的每种类型：编译文件、载入文件或字符串、编译eval、内核函数、用户函数。

单元测试以后也会把这个坑填上的，还请各位fork的朋友帮忙测试测试。

这里主要使用了两个测试文件，分别在php5.4.40+xhprof0.9.2、php7.0.0-dev+xhprof0.9.5下测试。

`xhprof_enable`主要是每个函数与过程都测试，其测试脚本为
[enable.php](https://github.com/Yaoguais/phpng-xhprof/blob/master/diff/enable.php)

测试结果分别为
[5.4.40-enable-192934.out](https://github.com/Yaoguais/phpng-xhprof/blob/master/diff/5.4.40-enable-192934.out)、
[7.0.0-dev-enable-011140.out](https://github.com/Yaoguais/phpng-xhprof/blob/master/diff/7.0.0-dev-enable-011140.out)

从文件对比上来看，结果相似度达到了100%，包括key的顺序，文件路径格式等。

`xhprof_sample_enable`主要是每隔0.1s进行函数采样，其测试脚本为
[sample.php](https://github.com/Yaoguais/phpng-xhprof/blob/master/diff/sample.php)

测试结果分别为
[5.4.40-sample-192805.out](https://github.com/Yaoguais/phpng-xhprof/blob/master/diff/5.4.40-sample-192805.out)、
[7.0.0-dev-sample-192843.out](https://github.com/Yaoguais/phpng-xhprof/blob/master/diff/7.0.0-dev-sample-192843.out)

从文件对比上来看，也是基本一致。

最后，帮助xhprof扩展完成了phpng版本。在看到输出一模一样的时候，别提有多高兴了！





