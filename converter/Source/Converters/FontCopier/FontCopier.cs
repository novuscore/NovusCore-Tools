using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Microsoft.Scripting.Hosting;

namespace Converter.Converters
{
   
    class FontConverter : BaseConverter
    {
        public override void Init()
        {

        }

        public override bool CanConvert(string pythonPath, ScriptScope scope, out string error)
        {
            error = "";
            string assetPath = GetAssetPath(pythonPath);
    
            if (!assetPath.EndsWith(".ttf"))
            {
                error = "We only convert .ttf files";
                return false;
            }

            return File.Exists(assetPath);
        }

        public override bool Convert(string pythonPath, ScriptScope scope, string outputDirectory, out string error)
        {
            error = "";
            string assetPath = GetAssetPath(pythonPath);
            outputDirectory = Path.Combine(outputDirectory);

            int subPathStartIndex = assetPath.IndexOf("client/Data/") + $"client/Data/".Length;
            string subPath = assetPath.Substring(subPathStartIndex);

            string destinationFilePath = Path.Combine(outputDirectory, subPath);
            Directory.CreateDirectory(Path.GetDirectoryName(destinationFilePath));

            try
            {
                File.Copy(assetPath, destinationFilePath, true);
                return true;
            }
            catch (Exception e)
            {
                error = e.Message;
            }

            return false;
        }
    }
}