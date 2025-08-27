# `idgen` usage

Create id file named `idfile.id`
```bash
./idgen -o idfile.id
```

Verify license for application with id "scope+gen"
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

# ID file structure

```json
{
	"mac_address" : "C8:60:00:6C:A7:34",
	"zynq_id" : "13833436209990006060",
	"apps" : [
		{
			"app_id" : "spectrum",
			"app_name" : "Spectrum Analyzer",
			"app_cheksum" : "b1631a20b6332b7e4655cb9e99922984"
		},
		{
			"app_id" : "scope+gen",
			"app_name" : "Oscilloscope & Generator",
			"app_cheksum" : "b1631a20b6332b7e4655cb9e99922984"
		},
		{
			"app_id" : "scope",
			"app_name" : "Oscilloscope",
			"app_cheksum" : "b1631a20b6332b7e4655cb9e99922984"
		},
		{
			"app_id" : "myScope",
			"app_name" : "MyScope",
			"app_cheksum" : "b1631a20b6332b7e4655cb9e99922984"
		}
	]
}
```
