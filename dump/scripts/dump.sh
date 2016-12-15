#!/usr/bin/env bash

mysqldump -u root -v $1 > $2
