*
*  ---  Make Script  ---
*

echo
echo "  Making Program"
echo

if {#} == 1
    if {1} == all
         set Make-All 1
    else
         echo "MAKE: Unknown Parameter."
         exit 1
    end
else
    set Make-All 0
end

set do-link 0

for Source-File in Port.Asm C.Port.CC Test.CC Errors.CC ErrorMsgs.Asm
    if {Make-All} == 1
         set do-comp 1
    else
         set do-comp 0
         echo "Checking... "{Source-File}
         makehelp {Source-File}
    end

    if {do-comp} == 1
         echo "     Compiling... "{Source-File}
         compile -e -w KEEP={FilePort}/objects/$ {Source-File}
         set do-link 1
    end
end

if {do-link} == 1
    linkit
end

unset Source-File Make-all do-comp do-link
