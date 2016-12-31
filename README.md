# MultiplayerExample
Simple multiplayer shmup for Kore using a combination of reliable and unreliable packets supplemented by client prediction.

# Running the example
The example supports up to two clients per server (i.e. three players overall). Clients can connect and disconnect at any time, but only receive future changes (for example, enemies that have been spawned earlier are not send on connect).

## Server
`MPShmup <serverport>`

## Client
`MPShmup <clientport> <serveradress> <serverport>`

# Implementation details

This example uses an authoritative server, which means that the server is always right should a conflict arise. For communication, it uses **unreliable pakets for regular updates**. These are send 20 times per second (or 10 per second if congestion is detected). In addition to that, **rare events are send individually in a reliable manner**.

In my experience, this dual approach works quite well. But an alternative design could send the last n rare events with each unreliable update as well in order to gain a slight reduction in transmission time.

## Time

The regular updates from the server contain the timestamp at which they have been send. On receving such an update, the clients add the half ping (which describes a round trip) to compensate for the time elapsed during transmission. Using this corrected value, **the clients can calculate an offset that can be used to translate between client and server timestamps**. To account for jitter, this offset is fed into a rolling average.

## Player controls and ship movement

Client input is send during the regular updates. Similarily, the server sends the new ship position of each player with every update. Due to the relatively high send rate, missing or out-of-order pakets are unnoticeable for the players.

However, the delay between player input and updated ship movement (one round trip) is noticeable, even in a LAN enviroment. This can be compensated for by using **client prediction**, i.e. updating the ship position directly on the client. In order to still respect the server's authority, the server updates are then then retroactivly incorporated into the clients result. This is done in a smoothed manner (differences are resolved over time, not by warping immediately) in order to make this process invisible to the players.

Similarily, the other player's inputs are also transmitted by the server in order to allow the clients to pre-calculate the other players next position (**opponent prediction**).

## Rockets

The event of firing a rocket is send using reliable pakets. Since they move in a linear fashion with constant parameters, no further information is necessary in order to keep them synchronized afterwards.

For the clients, prediction is employed here as well with the rocket position being adapted over time in cases where the firing moment is differing between them and the server reported values.

## Enemies

Enemies are spawned randomly by the server and are therefore not based on client input. Additionally, they are initially positioned outside of the screen. This means that they can be transmitted reliably without any client prediction means. Like the rockets, their movement is completely deterministic and does not have to be synchronized.

Predicting them, for example by synchronizing the random seed or by transmitting the future spawn events early in turn could allow the clients to cheat by visualizing the next spawn positions.

Because the enemies and rocket positions are converging after a short initialization the collisions between them can be acurately calculated on the clients as well and no "enemy hit"-event is used. If scoring is added later, one should calculate this on the server and add a correspoding event to be sure though.
