# `idgen` usage

create id file named idfile.id
```bash
./idgen -o idfile.id
```

verify license for application with id "scope+gen"
```bash
./idgen -i idfile.id -v lic.lic -a scope+gen
```

Options
```bash
idgen -o <ID file name> -i <ID file name>  -v <Licence file> -a <Application id>

	-o Specify output ID file name.
	-i Specify input ID file name.
	-v Specify input license file to verify
	-a Specify input application id to verify.
```
