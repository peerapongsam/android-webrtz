```
maven { url 'https://raw.github.com/peerapongsam/android-webrtz/maven' }
```

```
implementation 'org.webrtz:webrtz:67'
```

```
mvn install:install-file -Dfile=./libwebrtz.aar -DgroupId=org.webrtz -DartifactId=webrtz -Dversion=67 -Dpackaging=aar -DlocalRepositoryPath=./android-webrtz -DcreateChecksum=true
```