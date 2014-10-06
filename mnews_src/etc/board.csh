#!/bin/csh
# This is a sample C-shell script file for mnews board-mode.

# Board spool directory
set board_path  = "/usr/spool/board"
#set board_path  = "/var/board"

# Board group name
set board_group = "xxx/misc"

if (! -d $board_path/$board_group) then
	echo "Illegal board group "'"'$board_group'"'.
	exit 1
endif
if (-f $board_path/$board_group/.count) then
	set article = `cat $board_path/$board_group/.count`
	set article = `expr $article + 1`
else
	set article = 1
endif
if (-f $board_path/$board_group/$article) then
	echo "System error article $article already exists."
	cat > $board_path/$board_group/#$article
endif
cat > $board_path/$board_group/$article
chmod 644 $board_path/$board_group/$article
if (-f $board_path/$board_group/.count) then
	cp /dev/null $board_path/$board_group/.count
else
	touch $board_path/$board_group/.count
	chmod 666 $board_path/$board_group/.count
endif
echo "$article" >> $board_path/$board_group/.count
