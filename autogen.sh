echo "=============== running libtoolize --force --copy" &&
    libtoolize --force --copy &&
    echo "=============== running aclocal" &&
    aclocal &&
    echo "=============== running autoheader" &&
    autoheader &&
    echo "=============== running automake --add-missing --copy --foreign" &&
    automake --add-missing --copy --foreign &&
    echo "=============== running autoconf" &&
    autoconf &&
    echo "=============== done"
