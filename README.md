This is the latest implementation of [COWL](http://cowl.ws) for the
Firefox web browser. We've taken on the challenge of proving the COWL
source as a clean set of patches for bleeding-edge Firefox. Hence, at
the time of this writing all the features describe in the paper are
not yet available. We will keep working to make these available as
soon as possible. However, if you are interested in a slightly older
version of COWL, with a slightly less clean patch, please contact us
-- we would be happy to provide.


### Build COWL

To build COWL you need a `.mozconfig` file.  Here is an example of the
debug build with the `clang` compiler:

```
export CC="clang -fcolor-diagnostics -DCOWL_DEBUG"
export CXX="clang++ -fcolor-diagnostics -DCOWL_DEBUG"
ac_add_options --enable-application=browser
ac_add_options --enable-debug
ac_add_options --disable-optimize 
mk_add_options MOZ_OBJDIR=@TOPSRCDIR@/obj-debug
ac_add_options --with-ccache=/usr/bin/ccache
export COWL_DEBUG=1
```

If you just want an optimized non-debug version, here is an example
with GCC (latest clang on Arch Linux does not compile as of this
writing):

```
ac_add_options --enable-application=browser
mk_add_options MOZ_OBJDIR=@TOPSRCDIR@/obj
ac_add_options --with-ccache=/usr/bin/ccache
```

Once you have the `.mozconfig` file simply run `mach build`.

###  Running COWL

To run the customized Firefox browser simply execute `mach run`.
Checkout the examples at http://cowl.ws to start playing around.

### Troubleshooting

#### LWorkers missing?

The next set of patches will include our LWorkers. For now you can
play with an LWorker-like API built on top of iframes provided at
[http://cowl.ws/cowl.js](http://cowl.ws/cowl.js).
