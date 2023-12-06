project('mid', 'c')
dep_uuid = dependency('uuid')
dep_zmq = dependency('libzmq')
src = ['mid.c', 'sigtrap.c', 'util.c', 'JSON/cJSON.c']
executable('mid', src, dependencies : [dep_uuid, dep_zmq])
