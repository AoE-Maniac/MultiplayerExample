# MultiplayerExample
Simple multiplayer shmup for Kore using a combination of reliable and unreliable packets supplemented by client prediction.

# Running the example
The example supports up to two clients per server (i.e. three players overall). Clients can connect and disconnect at any time, but only receive future changes (for example, enemies that have been spawned earlier are not send on connect).

## Server
MPShmup <serverport>

## Client
MPShmup <clientport> <serveradress> <serverport>
