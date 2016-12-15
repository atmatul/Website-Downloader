#!/usr/bin/env bash

mysqladmin -u root create $2
mysqldump -u root -v $1 | mysql -u root -D $2