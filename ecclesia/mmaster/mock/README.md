# Ecclesia Mock Daemon

This package contains a binary for running a mock MachineMaster service. It it
will provide responses to most MachineMaster RPCs, but with responses supplied
based on a static set of mock protos.

Since the MachineMaster API is resource oriented the mock daemon is driven by
supplying it with definitions for a bunch of fake resource objects. For every
instance of a resource you want the daemon to report you must supply a query
response proto for that resource. The mock daemon will then use all of the mock
resources it is given to respond to Enumerate and Query RPCs.

The input format for the mocks that the daemon expects is a directory of text
protobufs. Each protobuf must be a message of type:

```text
Query${Resource}Response
```

with a filename of the form:

```text
${Resource}.${something}.textpb
```

The value of the `${something}` can be any (dotless) string and is not used by
the mock server. The mock author is free to use it to give each file a
descriptive identifier.

For example to add a new Firmware resource to the mock you would add a new file
called `Firmware.xxx.textpb` with a `QueryFirmwareResponse` text proto in it.
The daemon will then add that firmware to the set of ones that will be reported
in its `EnumerateFirmware` and `QueryFirmware` RPCs.

The mock server will load all of the text protos into memory at startup and will
use those values until it is terminated. This means that it will not
automatically pick up changes made to the directory of mocks. If you change the
mock files and want the server to use the new data you must restart it.

To start the server you must supply it with two flags: `--uds_path`, a path to a
Unix domain socket in the filesystem where the server will run, and
`--mocks_dir`, a path to the directory of text protos defining your mock
resources.
