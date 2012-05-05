#
# FullFAT Verification System
#

Originally I had intended to develop FullFAT from scratch using a complete test-driven
development model. Unfortunately I instantly came upon a lot of requests to have a working
version completed quickly, and my TDD process was soon scrapped.

Over the years, the need for some kind of verification and test system has become increasingly
apparent. So here it is!

The top-level utility simply searches for DLL/SO files in the tests folder. It then dynamically
loads these at run time.

This way its very simple to add new test modules. A test module can include any number of tests
but should be organised in someway logical manor. E.g. tests pertaining to certain API's. 
Unit tests for each module etc.

#
# Test Structure
#

Test are organised in the following way:

1. API Tests
==================================================
These are like all encompassing unit tests that verify the behaviour at an API level. These are
the most important tests, as they guarantee that FullFAT behaves at a "MACRO" level as a developer
is expecting.

2. FS Tests
==================================================
These tests are somehow unique, and based on specific use cases that have created problems in the past.
They do not follow any set pattern or rules, but are simply added whenever someone reports an issue.
All FS tests attempt to use the standard API's to an extreme level in an attempt to cause data corruption
on the disk. Verification here is more tricky, as it usually requires MD5 sums of files, or disk sectors.
It is therefore allowed to use FullFAT internals to determine correctness, even though these tests might
be broken in future version.

3. Unit Tests
==================================================
Every function in every module should be wrapped, and tested to the extreme. These don't include the API
functions, as they should already be covered.

4. Coverage
==================================================
This actually requires a different testing system, and will be added later.
