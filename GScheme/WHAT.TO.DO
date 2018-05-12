GScheme commands I need to implement
------------------------------------

SPECIAL FORMS
-------------
sequence <- use in cond and eval-from-editor (which means that instead of
      eval(getln) we do sequence(getln)
let
quote  <- special form to do the exact same thing as the '
(and add the QUOTE and ' list-etc. build-up routine.
delay & force
define (procedure args) ... the syntactic sugar for lambda
lambda listvar       ... reading all args into one list
set!

PRIMITIVES
----------
eval & apply (a matter of printing the variable to a cstring & using our eval)
cadr,caddr,cddr,cdddr,cadddr, etc. etc. ad naseum. simple stuff
equal?  <- symbol, number, and list of lists etc. comparison
(memq item list)  <- return sublist of the list beginning with the symbol
nth <- return nth item of a list
number?
symbol?
integer?
and
or
not
abs
square
sqrt
sin,asin
cos,acos
tan,atan
log,ln
set-car!,set-cdr!
append!
