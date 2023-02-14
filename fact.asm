func:
    LDE x
    LDC 0
    EQ
    SEL then else
    STP

then:
    LDC 1
    STP

else:
    LDE f
    LDC 1
    LDE x
    SUB
    LDE f
    AP
    LDE x
    MUL
    STP

main:
    LDF func (f x)
    ST f
    LDE f
    LDC 7
    LDE f
    AP
    STP
