#!/bin/sh

cur=`dirname $0`;
cd $cur;
cur=$PWD

rm -f *.rpm 2>/dev/null

src=$(cd ..;pwd)

version=`grep "define OD_VERSION" $src/php_odus.h  | awk '{print $3}' | sed 's/"//g'`

topdir=/tmp/odus_rpmbuild
rm -rf $topdir 2>/dev/null

pkgname="odus-$version"

des="$topdir/SOURCES/$pkgname.tgz"

mkdir -p $topdir/{SRPMS,RPMS,BUILD,SOURCES,SPECS}
mkdir -p $topdir/$pkgname

cp -r $src/* $topdir/$pkgname

rm -rf $topdir/$pkgname/rpm 2>/dev/null

cd $topdir/$pkgname
make clean 1>/dev/null 2>&1
cd $cur

rm -rf $topdir/$pkgname/modules 2>/dev/null

echo "Creating source tgz ..." && \
cd $topdir && \
tar czf $des $pkgname 2>/dev/null && \
cd $cur

export VERSION=$version
export DATE=`date`
export SOURCE=$des

specfile="$topdir/SPECS/php-odus-zb.spec"

echo "Generating spec file ... " && \
./replace ./php-odus-zb.spec.tpl $specfile

echo "%changelog" >>$specfile

cat $src/ChangeLog >>$specfile

echo "Building rpm ..." && \
rpmbuild --define="_topdir $topdir" -ba $specfile && \
cp $topdir/SRPMS/*.rpm . && \
cp $topdir/RPMS/*/*.rpm . && \
rm -rf $topdir 
