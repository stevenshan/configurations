#!/bin/bash

# first clone this repository

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ~/

sudo apt update
sudo apt install -y tmux vim git mosh
sudo apt upgrade -y

ln $DIR/dotfiles/.bashrc .bashrc
ln $DIR/dotfiles/.tmux.conf .tmux.conf
ln $DIR/dotfiles/.vimrc .vimrc

# install vundle (vim plugin manager)
git clone https://github.com/VundleVim/Vundle.vim.git ~/.vim/bundle/Vundle.vim
vim +PluginInstall +qall

git config --global user.email "me@stevenshan.com"
git config --global user.name "Steven Shan"

