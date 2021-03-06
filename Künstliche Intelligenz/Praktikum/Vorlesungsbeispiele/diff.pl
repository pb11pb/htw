:- [simplify].
diff(F,X,SDF) :- d(F,X,DF), s(DF,SDF).
d(X,X,1).
d(C,X,0) :- atomic(C), C \== X.
d(-F,X,-DF) :- d(F,X,DF).
d(C*F,X,C*DF) :- d(C,X,0), d(F,X,DF).
d(F+G,X,DF+DG) :- d(F,X,DF), d(G,X,DG).
d(F-G,X,DF-DG) :- d(F,X,DF), d(G,X,DG).
d(F^N,X,SN*F^N1*DF) :- s(N,SN),number(SN),N1 is SN-1,d(F,X,DF).
d(sin(F),X,cos(F)*DF) :- d(F,X,DF).
d(cos(F),X,(-sin(F))*DF) :- d(F,X,DF).
d(log(F),X,DF/F) :- d(F,X,DF).