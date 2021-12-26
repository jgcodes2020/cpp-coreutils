# cxx-coreutils
There seems to be a bit of a controversy.

Everyone calls Linux "Linux".  
GNU proponents call it "GNU/Linux", since some of the core software running atop most Linux distributions is GNU.

As of today, there is definitely enough software to outright replace any trace of GNU: 

| GNU software   | Non-GNU replacement |
| :------------- | :------------------ |
| bash           | zsh                 |
| emacs          | vim, nvim           |
| glibc          | musl, uclibc        |
| gcc, g++, etc. | clang, clang++      |
| libstdc++      | libc++              |
| coreutils      | busybox             |

The thing is, I read somewhere that although busybox packs everything together, it doesn't support as many extensions to POSIX syntax. So I wanted to reimplement the coreutils. I know it runs on C++, but as long as there's some kind of C++ standard library installed, it should work.

## Cross-platformness
Ideally, you could, but since I'm going for POSIX, some commands by nature are not portable. In addition, I do not have a Mac to test on.