using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;
using System.IO;

namespace csharp_aes_encryptor
{
    abstract class Encryptor
    {
        abstract public byte[] Encrypt(byte[] data, byte[] entropy);
        abstract public byte[] Decrypt(byte[] encryptedData, byte[] entropy);
    }

    abstract class StreamCryptor
    {
        public static Stream Encrypt(Encryptor en, Stream inStream, byte[] entropy)
        {
            using (MemoryStream ms = new MemoryStream())
            {
                inStream.CopyTo(ms);
                byte[] protectedData = en.Encrypt(ms.ToArray(), entropy);
                return new MemoryStream(protectedData);
            }
        }

        public static Stream Decrypt(Encryptor en, Stream inStream, byte[] entropy)
        {
            using (MemoryStream ms = new MemoryStream())
            {
                inStream.CopyTo(ms);
                byte[] unprotectedData = en.Decrypt(ms.ToArray(), entropy);
                return new MemoryStream(unprotectedData);
            }
        }
    }

    class DpapiEncryptor : Encryptor
    {
        public override byte[] Encrypt(byte[] data, byte[] entropy)
        {
            return ProtectedData.Protect(data, entropy, DataProtectionScope.LocalMachine);
        }

        public override byte[] Decrypt(byte[] encryptedData, byte[] entropy)
        {
            return ProtectedData.Unprotect(encryptedData, entropy, DataProtectionScope.LocalMachine);
        }
    }

    class AesEncryptor : Encryptor
    {
        public override byte[] Encrypt(byte[] data, byte[] entropy)
        {
            if (data == null || data.Length <= 0)
                throw new ArgumentNullException("encryptedData");
            if (entropy == null || entropy.Length <= 0)
                throw new ArgumentNullException("entropy");

            using (AesCryptoServiceProvider aesEncryptor = new AesCryptoServiceProvider())
            {
                aesEncryptor.Mode = CipherMode.CBC;
                aesEncryptor.Key = entropy;
                aesEncryptor.Padding = PaddingMode.None;

                byte[] iv = { 0x46, 0xb6, 0x02, 0x6a, 0x99, 0x21, 0x90, 0xde, 0xfd, 0xf4, 0x5b, 0x42, 0x94, 0xde, 0xa6, 0x23 };
                aesEncryptor.IV = iv;
                byte[] encrypted;
                using (ICryptoTransform encryptor = aesEncryptor.CreateEncryptor(aesEncryptor.Key, aesEncryptor.IV))
                {

                    // Create the streams used for encryption.
                    using (MemoryStream msEncrypt = new MemoryStream())
                    {
                        using (CryptoStream csEncrypt = new CryptoStream(msEncrypt, encryptor, CryptoStreamMode.Write))
                        {
                            csEncrypt.Write(data, 0, data.Length);
                            csEncrypt.FlushFinalBlock();
                            encrypted = msEncrypt.ToArray();
                        }
                    }
                }
                return encrypted;
            }
        }


        public override byte[] Decrypt(byte[] encryptedData, byte[] entropy)
        {
            if (encryptedData == null || encryptedData.Length <= 0)
                throw new ArgumentNullException("encryptedData");
            if (entropy == null || entropy.Length <= 0)
                throw new ArgumentNullException("entropy");

            using (AesCryptoServiceProvider aesEncryptor = new AesCryptoServiceProvider())
            {
                aesEncryptor.Mode = CipherMode.CBC;
                aesEncryptor.Key = entropy;
                aesEncryptor.Padding = PaddingMode.None;
                
                byte[] iv = { 0x46, 0xb6, 0x02, 0x6a, 0x99, 0x21, 0x90, 0xde, 0xfd, 0xf4, 0x5b, 0x42, 0x94, 0xde, 0xa6, 0x23 };
                aesEncryptor.IV = iv;

                using (ICryptoTransform decryptor = aesEncryptor.CreateDecryptor(aesEncryptor.Key, aesEncryptor.IV))
                {
                    byte[] decrypted;
                    // Create the streams used for decryption.
                    using (MemoryStream msDecrypt = new MemoryStream())
                    {
                        using (CryptoStream csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Write))
                        {
                            decrypted = new byte[encryptedData.Length];
                            csDecrypt.Write(encryptedData, 0, encryptedData.Length);
                            csDecrypt.FlushFinalBlock();
                            decrypted = msDecrypt.ToArray();
                            return decrypted;
                        }
                    }
                }
            }
        }
    }
}
