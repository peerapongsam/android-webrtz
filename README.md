# WebRTZ
This repo fork from google webRTC and change package name to webrtz to avoid conflict with some library

# Install

```
repositories {
    maven { url 'https://raw.github.com/peerapongsam/android-webrtz/maven' }
}
```

```
dependencies {
    implementation 'org.webrtz:webrtz:67'
}
```


# Create Local maven package
```
mvn install:install-file -Dfile=./libwebrtz.aar -DgroupId=org.webrtz -DartifactId=webrtz -Dversion=67 -Dpackaging=aar -DlocalRepositoryPath=./android-webrtz -DcreateChecksum=true
```
