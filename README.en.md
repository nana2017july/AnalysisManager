[Japanese](https://github.com/nana2017july/AnalysisManager/) | [English](https://github.com/nana2017july/AnalysisManager/blob/master/README.en.md)
# Library for analyzing and replacing Http body (using in Apache Module)

It was created mainly for Apache Module devleop , but basically an auxiliary library for APR. <br>
HTTP body part can be easily analyzed and replaced by creating a derived this library class. <br>

we can analyze multipart body, and so on.

now making english version...


# Apache Module library for analyzing and replacing body parts
It is created with the image of using mainly Apache Module etc., but basically an auxiliary library for APR. <br>
HTTP body part can be easily analyzed and replaced by creating a derived class. <br>
<br>

**Problems in handling apr_bucket_brigade **<br>
apr_bucket_brigade has a problem that it cannot handle strings continuously, and the Apache Module hook function is also called intermittently. <br>
It seems that there are many people who misunderstand that the hook function is called only once for each request or response. <br>
However, in reality, the HTTP body part is divided into briagde and the hook function is called many times, so the character string is intermittent. <br>
For example, if there is an HTTP body part "012345678", the first time "012" is passed to brigade to the hook function, <br>
The second time, "345678" is passed to the hook function. <br>
If you try to search for the string "23" in this case, it will break at "2", so the hook function will come to the brigade that comes next time
You must check if "3" is included. <br>
This is quite difficult. <br>
<br>
This library is a useful tool that allows you to work without worrying about brigade's intermittentness. <br>
<br>

## What this library can do
When you create a derived class, you can do the following without worrying about the intermittentness of brigade. <br>
* Search HTTP body string
* HTTP body string replacement
* Can be used for both Input and Output filters
* Since bucket is used as it is, it can be processed without consuming memory resources.
* There is an Apache Module sample that uses the library to perform partial matching and HTML tag replacement.

The provided Apaceh Module is a sample using the library.
<br>

## Introduction
The license is Apache License, Version 2.0. <br>
<br>
** [Function / Class documentation (doxygen)] ** <br>
https://nana2017july.github.io/AnalysisManager/ <br>
<br>


## change history
ver.1.0 Create new <br>

## Operating environment
** [Compiler] ** <br>
It is required that it can be used in the following compilation environment. <br>
* Microsoft Visual Studio 2019
* CentOS7 g ++ (4.8.5)
<br>
<br>

** [Used] ** <br>
* Apache Httpd (2.4.39 (Win64), 2.4.6 (CentOS))
* C language

Install Apache 2.4 in your environment. <br>
<br>
<br>

** [Microsoft Visual Studio Build Supplement] ** <br>
For Microsoft Visual Studio, do the following before building.
* Launch "mod_replace_content.sln"
* Open project settings and add includes folder in Apache Httpd folder to "C / C ++"-> "Additional include directories".
* Open the project settings and add the libs folder in the Apache Httpd folder to "Linker"-> "Additional library directory".
* Open project settings and add "PATH =% PATH%; bin folder in Apache Httpd folder" to "Debug"-> "Environment"

After building, copy the created "mod_replace_content.so" to modules in the Apache Httpd folder and add the settings to httpd.conf
Please start httpd. <br>
I will show you how to write httpd.conf later. <br>
<br>
* If you want to run the test, start Ågtest \ test_am \ test_am.slnÅh and make the same settings as above.
<br>

** [CentOS build supplement] ** <br>
For CentOS, build using the following procedure.
* Open common.mk and check if APR_LIB_DIR at the top matches your environment, and correct it if necessary.
* Execute make test_am and confirm that there are no errors.
* make all
* make install

Since it has been copied to the Apache Module above, add the settings to httpd.conf before starting httpd. <br>
I will show you how to write httpd.conf later. <br>

** Targets available ** <br>

| ** Target ** | ** Description ** |
| --- | --- |
| make all | Build the Apache Module sample. mod_replace_content.so is created. |
| make clean | Remove all generated files such as .o |
| make test_am | Build and run CAManagerManager related tests. |
| make check_apxs_vars | Displays the apxs family values defined in the makefile. |
| make install | Install the generated .so into the Apache Module. |
| make start | Start httpd. |
| make restart | Restart httpd. |
| make stop | Stop httpd. |

<br>


## httpd.conf example for using Apaceh Module sample
```Apache
# Load created module
LoadModule replace_content_module modules / mod_replace_content.so
# Output filter setting
SetOutputFilter REPLACE_CONTENT_OUTPUTFILTER
# Input filter setting
SetInputFilter REPLACE_CONTENT_INPUTFILTER
Header set Last-Modified "Sat, 19 Apr 2014 21:53:07 GMT"
# replace content setting
ReplaceContent "tag: true" "head" "<banking>"
```
Input filter and Output filter work independently. It is a sample that works even if only one is set. <br>
<br>


### Output filter sample description
Set OutputFilter and ReplaceContent directive. <br>
The format of ReplaceContent is as follows. <br>
`` `Apache
ReplaceContent partial_match | tag [: true | false] target replacement
`` `
| ** Item ** | ** Description ** |
| --- | --- |
| Item 1 | partial_match ... Specifies partial match replacement. <br> tag ... Specify HTML tag replacement. If true is specified after the tag, the tag itself is replaced. false is inserted at the next position of the tag. |
| Item 2 | Replacement target string |
| Item 3 | Replaced character string |


* example1 <br>
ReplaceContent "partial_match" "a" "x" <br>
Input HTTP body: "123a45" <br>
Result HTTP body: "123x45" <br>

* example2 <br>
ReplaceContent "tag: false" "head" "\<meta \>" <br>
Input HTTP body: "\<head \> \</head \>" <br>
Result HTTP body: "\<head \> \<meta \> \</head \>" <br>
<br>


### Input filter sample description
It works just by setting the input filter.

* Operational content <br>
It reacts only during Multipart request processing and outputs parameters other than files to error.log.

```html: sample
<form method = "POST" action = "test.html" enctype = "multipart / form-data">
<input type = "text" name = "me; ssage" value = "He; llo" /> <br>
<input type = "text" name = "test" value = "testvalue" /> <br>
<input type = "file" name = "file" /> <br>
<input type = "submit" value = "SUBMIT" />
</ form>
```

```html: Log output result
[Mon Aug 12 14: 53: 45.000255 2019] [: notice] **** paramsTable [me; ssage] = He; llo \ n
[Mon Aug 12 14: 53: 45.000255 2019] [: notice] **** paramsTable [test] = testvalue \ n
```
<br>


## Brief description of class <br>
Although it is C language, it uses files and structs well, and operates close to C ++ class. <br>
There are some rules for using classes that must be followed. <br>

* Class is expressed by struct.
* The order of members of class struct is strict. Be sure to define AcCThisIsClass at the beginning.
* The class method name must start with the class name, and the method name is concatenated with an underscore.
* The first argument of a class method is always a pointer to the target class.
* Implement private methods by making them static functions in the .c file.
* Private variables are defined in the struct of "class name_Super" and hidden from the user.
* The header file beginning with _ is private and must not be included by the user.

** Description example **
```cpp
// The class's public definition. The definition inside the class is hidden and defined in the .c file.
typedef struct CBucketController {
AcCThisIsClass * thisIsClass;
} CBucketController;

// class method declaration
void CBucketController_exportModifiedBrigadeToBrigade (CBucketController * p_this, apr_bucket_brigade * outbb);
```


...now creating document...



