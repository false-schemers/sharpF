# Scheme Library Definitions
                         
This directory contains Scheme Library Definition (SLD) files to be used with IntL and SIOF interpreters.
None of the files contain any definitions or expressions -- all of them just import and then re-export the necessary bindings from
the built-in `(sharpf base)` library providing all supported R7RS-small procedures and syntax forms.

NB: `(scheme r5rs-null)` definitions contained in `r5rs-null.sld` are used internally to assemble bindings 
for the `(null-environment 5)` environment.
