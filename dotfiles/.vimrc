set nocompatible
set number
set ruler

set showmatch " highlight matching braces
set equalalways " split windows equal size

set expandtab
set tabstop=4
set shiftwidth=4

syntax on " syntax highlighting

" Strip whitespace from end of lines when writing file
 autocmd BufWritePre * :%s/\s\+$//e

 colorscheme desert

au BufRead,BufNewFile *.sml set shiftwidth=2
au BufRead,BufNewFile *.sml set tabstop=2
au BufRead,BufNewFile *.sml set softtabstop=2

set title  "Set window title to file
set hlsearch  "Highlight on search
set ignorecase  "Search ignoring case
set smartcase  "Search using smartcase
set incsearch  "Start searching immediately

set scrolloff=5  "Never scroll off
set wildmode=longest,list  "Better unix-like tab completion
set clipboard=unnamed  "Copy and paste from system clipboard
set lazyredraw  "Don't redraw while running macros (faster)
set wrap  "Visually wrap lines
set linebreak  "Only wrap on 'good' characters for wrapping
set backspace=indent,eol,start  "Better backspacing
set linebreak  "Intelligently wrap long files
set ttyfast  "Speed up vim
set nostartofline "Vertical movement preserves horizontal position

" Use jk for escape
inoremap jk <esc>
"inoremap <esc> <nop>

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" Vundle Stuff
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

set nocompatible              " be iMproved, required
filetype off                  " required

" set the runtime path to include Vundle and initialize
set rtp+=~/.vim/bundle/Vundle.vim
call vundle#begin()
" alternatively, pass a path where Vundle should install plugins
"call vundle#begin('~/some/path/here')

" let Vundle manage Vundle, required
Plugin 'VundleVim/Vundle.vim'

Plugin 'christoomey/vim-tmux-navigator'

" All of your Plugins must be added before the following line
call vundle#end()            " required
filetype plugin indent on    " required
