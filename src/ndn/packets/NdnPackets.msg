//
// NDN-IoT-RL - NDN Packet Definitions
// Plain OMNeT++ packets (no INET dependency); carried over UDP by NdnUdpTransport
// via inet::cPacketChunk at the overlay boundary.
//

namespace ndniot;

// NDN Packet Types
enum NdnPacketType {
    NDN_INTEREST = 0;
    NDN_DATA = 1;
    NDN_NACK = 2;
}

// Interest Packet Selector
struct InterestSelector {
    int minSuffixComponents = -1;
    int maxSuffixComponents = -1;
    bool mustBeFresh = false;
}

// NACK Reason Codes
enum NackReason {
    NACK_NONE = 0;
    NACK_CONGESTION = 50;
    NACK_DUPLICATE = 100;
    NACK_NO_ROUTE = 150;
}

//
// Base NDN Packet
//
packet NdnPacket {
    int packetType @enum(NdnPacketType);
    string name;                    // NDN name (e.g., /wsn/area1/temp/node3)
    simtime_t timestamp;            // Packet creation timestamp
    int hopCount = 0;               // Number of hops traversed
    string forwardingHint = "";     // Forwarding hint for routing
    string appPayload = "";         // Opaque coordination payload (e.g. cache digest)
}

//
// Interest Packet
//
packet InterestPacket extends NdnPacket {
    packetType = NDN_INTEREST;

    int nonce;                        // Random nonce for loop detection
    simtime_t interestLifetime = 4.0; // Interest lifetime (default 4 seconds)
    InterestSelector selectors;

    string contentType = "";        // Content class (sensor, config, video, ...)
    int priority = 0;               // Priority (0=normal, 1=high, 2=critical)
}

//
// Data Packet
//
packet DataPacket extends NdnPacket {
    packetType = NDN_DATA;

    string content;                  // Actual content/payload
    int contentLength;               // Content size in bytes
    simtime_t freshnessPeriod = 0.0; // How long data is fresh (0 = always stale)
    simtime_t originTime;            // Content generation time at producer (for age metric)

    int finalBlockId = -1;           // Last segment ID (for segmentation)
    string contentType = "";         // Content class

    // Security (kept minimal; verification is parameter-gated and off by default)
    bool isSigned = false;
    string signature = "";
    string signerId = "";
    simtime_t signatureTime;

    // Cache control
    bool isCacheable = true;         // Can be cached
    int cacheHits = 0;               // Number of cache hits (for tracking)
    int lcdDepth = -1;               // Hops since cache/producer hit (-1 = unset; used by LCD policy)
    bool servedFromCache = false;    // Set when this copy was served by a CS (remote-hit metric)
    // Number of Content Stores that have answered with this copy. Each such
    // hand-off lets the next cache restart FreshnessPeriod from its own
    // arrival time, so this counts the freshness re-stamps the copy has
    // collected and is the M of the staleness-amplification model.
    int restamps = 0;
    // The lifetime the publisher declared, carried unmodified end to end. The
    // clock-free realization rewrites freshnessPeriod at every cache so that
    // it holds the residual lifetime, so the nominal value has to travel
    // separately for the staleness test to stay on one basis. Nothing in the
    // forwarding or caching path reads this field; the consumer uses it for
    // measurement only.
    simtime_t nominalFreshness = 0.0;
}

//
// NACK Packet
//
packet NackPacket extends InterestPacket {
    packetType = NDN_NACK;
    int reason @enum(NackReason);
    string reasonText = "";
}
