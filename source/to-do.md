
in order to fix the complexity of having groups i should make it so that groups can store pairtokens and rangetokens
to implement this these groups should be ordered by priority and each group should take away tokens from the main cache to store within themselves.
after this is done they should replace the group start and group end tokens with a group token that points to which group needs to be resolved.

- make the "value" attribute of the PairToken struct a union that allows long, int, and void pointer types. this way it can have a void pointer to the group in question.

- groups should also keep track of the presence of dice so that it can determine if the result changes or if it's a fixed value. fixed values don't need to be run several times meaning we can stash away the result onto the group itself.