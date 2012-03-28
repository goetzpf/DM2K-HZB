# define a single space
empty:=
space:= $(empty) $(empty)

# convert space separated list of words to colon separated
list_to_searchpath = $(subst $(space),:,$(strip $1))
