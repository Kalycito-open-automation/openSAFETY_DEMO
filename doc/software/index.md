Software Manual {#page_idxsoftware}
============

[TOC]

This chapter provides details about the software which is executed on the
application processor. Basically the software consists of the following parts:
- \ref group_applications
- \ref group_libpsi
- \ref group_libpsicommon

The build steps for the software vary depending on the platform the software is
cross compiled. See the \ref page_ugindex for more information about each build
step.

All software parts including the interface libraries which are running on the
application processor provide unit tests in the folder `src/unittest`.

# POWERLINK interface driver (PCP)  {#sect_sw_pcp}
Documentation of the software running on the POWERLINK processor (PCP) is
not provided in this section as the end user has no need to carry out any changes
on the PCP itself.

Due to the fact that no initialization procedure between the PCP and the application
is implemented, it is needed to carry out configuration changes on the PCP at compile
time.

> This reduces the code for initialization and parametrization at the application
> and its user library.
