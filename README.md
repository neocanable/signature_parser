# Signature Parser
for parse class file's signature attributes and generic type

https://docs.oracle.com/javase/specs/jvms/se15/html/jvms-4.html#jvms-4.7.9.1

####example:

```shell

[method signature]: <U:Ljava/lang/Object;R:Ljava/lang/Object;>(Lorg/reactivestreams/Publisher<+TU;>;Lio/reactivex/rxjava3/functions/BiFunction<-TT;-TU;+TR;>;)Lio/reactivex/rxjava3/core/Flowable<TR;>;
[formal type parameters]: <U extends java/lang/Object, R extends java/lang/Object>
[parameter type]: org/reactivestreams/Publisher<? extends U>
[parameter type]: io/reactivex/rxjava3/functions/BiFunction<? super T & java.lang.Object, ? super U & java.lang.Object, ? extends R>
[return type]: io/reactivex/rxjava3/core/Flowable<R>
```

