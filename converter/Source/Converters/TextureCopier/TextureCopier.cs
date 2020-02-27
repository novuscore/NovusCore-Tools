using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Microsoft.Scripting.Hosting;

namespace Converter.Converters
{
   
    class TextureCopier : BaseConverter
    {
        string[] supportedFormats =
        {
            ".jpg",
            ".jpeg",
            ".png",
            ".bmp",
            ".tga"
        };

        public override void Init()
        {

        }

        public override bool CanConvert(string pythonPath, ScriptScope scope, out string error)
        {
            error = "";
            string assetPath = GetAssetPath(pythonPath);

            bool isSupported = false;
            foreach(string supportedFormat in supportedFormats)
            {
                if (assetPath.EndsWith(supportedFormat))
                {
                    isSupported = true;
                    break;
                }
            }

            if (!isSupported)
            {
                error = "We only copy files with these extensions: " + supportedFormats.ToString();
                return false;
            }

            return File.Exists(assetPath);
        }

        public override bool Convert(string pythonPath, ScriptScope scope, string outputDirectory, out string error)
        {
            error = "";
            string assetPath = GetAssetPath(pythonPath);

            outputDirectory = Path.Combine(outputDirectory, "textures");

            int subPathStartIndex = assetPath.IndexOf("client\\Data\\") + "client\\Data\\".Length;
            string subPath = assetPath.Substring(subPathStartIndex);

            string destinationFilePath = Path.Combine(outputDirectory, subPath);
            Directory.CreateDirectory(Path.GetDirectoryName(destinationFilePath));

            try
            {
                File.Copy(assetPath, destinationFilePath, true);
                return true;
            }
            catch(Exception e)
            {
                error = e.Message;
            }

            return false;
        }
    }
}