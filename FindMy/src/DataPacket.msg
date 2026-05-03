import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3AddressTag;
import inet.common.geometry.Geometry;
namespace inet;

class DataPacket extends FieldsChunk
{
    // Type-0 = AirTag location
    // Type-1 = Airtag lookup request 
    // Type-2 = Airtag lookup response
    // Type-3 = Iphone location
    // Type-4 = Iphone lookup request
    // Type-5 = Iphone lookup response
    // Type-6 = Friendly Sharing Location 
    int type;
    string deviceName;
    L3Address ip;
    int isDeviceFound;
    Coord deviceLocation;
    string textMessage;
}