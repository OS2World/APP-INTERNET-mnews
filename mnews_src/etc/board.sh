#!/bin/sh
# This is a sample B-shell script file for mnews board-mode.

# Board spool directory
board_path="/usr/spool/board"
#board_path="/var/board"

# Board group name
board_group="xxx/misc"

if [ ! -d $board_path/$board_group ]; then
	echo "Illegal board group "'"'$board_group'"'.
	exit 1
fi
if [ -f $board_path/$board_group/.count ]; then
	article=`cat $board_path/$board_group/.count`
	article=`expr $article + 1`
else
	article=1
fi
if [ -f $board_path/$board_group/$article ]; then
	echo "System error article $article already exists."
	cat > $board_path/$board_group/#$article
fi
cat > $board_path/$board_group/$article
chmod 644 $board_path/$board_group/$article
if [ -f $board_path/$board_group/.count ]; then
	cp /dev/null $board_path/$board_group/.count
else
	touch $board_path/$board_group/.count
	chmod 666 $board_path/$board_group/.count
fi
echo "$article" >> $board_path/$board_group/.count
