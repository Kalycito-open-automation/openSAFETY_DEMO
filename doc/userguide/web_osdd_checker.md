Checking and Securing OSDD files {#page_web_osdd_checker}
============

[TOC]

This page provides information about the Web OSDD Checker tool.
An OSDD file has to be valid in respect to the OSDD specification and it must have a valid CRC checksum
to be accepted by a design tool. The Web OSDD Checker enables users to do the  checks and checksum
calculation for own / customized OSDD files.


# Web OSDD Checker {#sect_web_osdd_checker}

The Web OSDD Checker can be found at http://opensafetydemo.sourceforge.net/web_OSDD_check/upload.php.

To check and secure own / customised files, the following steps need to be carried out:


 - Enter your e-mail address and select the OSDD files for checking and securing.
 - Upload the files. Files which are uploaded successfully will be checked and secured.
 - This procedure can take up to 15 minutes.
 - The processed files and the results of the check will be sent to the provided e-mail address. Check your spam mail, in case an expected respond takes too long.

 ![Web OSDD Checker - procedure](web_osdd_checker_procedure.png)
@image latex web_osdd_checker_procedure.png "Web OSDD Checker - procedure" width=0.75\textwidth

# Limitations {#sect_web_osdd_checker_limitations}

Nevertheless there are some limitations:

 - The number of files and the overall file size is limited.
 - Only valid XML files can be processed.
 - Only files containing the vendorID "02:12:34" can be processed.