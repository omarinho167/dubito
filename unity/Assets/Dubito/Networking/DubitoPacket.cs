using Mirror;

namespace Dubito.Networking
{
    // Mirror serializes only this envelope; ProtocolCodec owns the actual wire contract.
    public struct DubitoPacket : NetworkMessage
    {
        public byte[] Payload;
    }
}
