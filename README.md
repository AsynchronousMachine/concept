# AsynchronousMachine
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)

Refer to *src/testcases* in git branch ***master***!

# TODOs

* More testing

# Used open source projects

* [Boost](http://www.boost.org)
* [Threading Building Blocks](http://www.threadingbuildingblocks.org)
* [RapidJSON](http://rapidjson.org/)
* [spdlog](https://github.com/gabime/spdlog)

Thanks to all the guys who keep this projects up and running!


# Building on Debian

Required packages:

* [libboost-all-dev](https://packages.debian.org/de/stretch/libboost-all-dev)
+ [libtbb-dev](https://packages.debian.org/de/stretch/libtbb-dev)

# Getting the githook up and running

First, install [`git-hooks`](https://github.com/icefox/git-hooks):

```shell
me@my-machine:~$ sudo bash -c 'curl -RLSsf1o /usr/local/bin/git-hooks https://github.com/icefox/git-hooks/raw/master/git-hooks && chmod +x /usr/local/bin/git-hooks'
```

Second, let `git-hooks` manage the git hooks:

```shell
me@my-machine:~/My/Repo/Dir: git hooks --install
```

