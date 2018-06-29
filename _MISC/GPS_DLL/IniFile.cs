using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;


/* Simple INI file class, using Regular Expressions.
 * Easy to use, can be used for all sorts of file record needs.
 * 
 * Author: Aaron Bregger
 * Date: 4/15/09
 * 
 * Distributed under GNU GPL:

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

namespace CWP
{
    public class IniFile
    {
        /// <summary>
        /// The full path and name of the ini file.
        /// Example: 
        /// "C:\folder\file.ini"
        /// </summary>
        public string FilePath { get; set; }
        
        /// <summary>
        /// A dictionary of sections within the INI file.
        /// </summary>
        public IniSections Sections { get; set; }

        /// <summary>
        /// A dictionary of parameters not contained within a specific section.
        /// </summary>
        public IniParameters Parameters { get; set; }

        /// <summary>
        /// Creates a new, empty, INI file.
        /// </summary>
        public IniFile()
        {
            Sections = new IniSections();
            Parameters = new IniParameters();
            FilePath = "";
        }

        /// <summary>
        /// Attempts to save the INI file record to its associated physical file.
        /// </summary>
        public void SaveFile()
        {
            if (String.IsNullOrEmpty(FilePath))
                throw new InvalidOperationException("This INI record has no associated file.");
            SaveFile(FilePath);
        }

        /// <summary>
        /// Attempts to save the INI file record to the specified physical 
        /// file. It is created if it does not exist.
        /// </summary>
        /// <param name="filePath">
        /// The full path to the file which will be created or overwritten.
        /// </param>
        public void SaveFile(string filePath)
        {
            //file is not appended to. Default encoding is UTF8
            System.IO.TextWriter tw = new System.IO.StreamWriter(filePath, false, Encoding.UTF8);

            //the first written parameters are the ones without a heading
            foreach (KeyValuePair<string, string> parameter in this.Parameters)
            {
                tw.WriteLine("{0}={1}", parameter.Key.ToUpper(), Escape(parameter.Value));
            }

            //there may be multiple sections, so loop through all of them
            foreach (IniSection section in this.Sections.Values)
            {
                tw.WriteLine();
                tw.WriteLine("[{0}]", section.SectionName);

                //most INI conventions state that keys are case insensitive, and this
                //will convert them to uppercase for clarity.
                foreach (KeyValuePair<string, string> parameter in section.Parameters)
                {
                    tw.WriteLine("{0}={1}", parameter.Key.ToUpper(), Escape(parameter.Value));
                }
            }
            tw.Close();
        }
        
        //a pattern gifted unto thee by the regex gods
        private const string INI_SETTING_PATTERN = @"
                    ^\s*                        #accounts for leading spaces
                    (?<Key>                     #start of key group
                        [A-Za-z0-9_-]+          #key is made up of letters, digits and _ or -
                    )                           
                    \s*                         #accounts for spaces before delimiter                
                    [=:]                        #delimiter may be a : or =
                    \s*                         #spaces after delimiter
                    (?<Value>                   #value group
                        [^\s#;:=]+?             #at least one non-whitespace or comment character, lazy
                        ((?<=\\)[=:]|[^=:])*?   #doesnt match unescaped = or :, is lazy to exclude whitespace and comments
                    )                           
                    \s*                         #account for spaces before comment or after value
                    ((?<!\\)[#;].*)?            #a comment starting with # or ;
                    $                           #end of string";
                    
        /// <summary>
        /// Creates a new IniFile that is loaded from a path.
        /// </summary>
        /// <param name="filePath">Loads from Path</param>
        public IniFile(string filePath) : this()
        {

            string[] iniLines = File.ReadAllLines(filePath, Encoding.UTF8);
            IniParameters currentSection = Parameters;
            Regex settingPattern = new Regex(INI_SETTING_PATTERN, 
                RegexOptions.IgnorePatternWhitespace | 
                RegexOptions.Compiled);
            foreach (string line in iniLines)
            {
                //see above for amazing regex skill. Not bad for a first timer IMO.
                
                Match settingMatch = settingPattern.Match(line);

                Match sectionMatch = Regex.Match(line, @"^\s*\[(?<Name>[^\[\]]+)\]\s*([#;].*)?$");

                bool lineIsEmpty = Regex.IsMatch(line, @"^\s*([#;].*)?$");

                //if it is a setting it is added to the current section
                //if it is a section the current section is changed and added to the file
                //otherwise, if the line is not blank or commented an error is thrown
                if (settingMatch.Success)
                {
                    //keys are stored as uppercase
                    string key = settingMatch.Groups["Key"].Value.ToUpper();
                    string value = settingMatch.Groups["Value"].Value;

                    currentSection.Add(key, Unescape(value));
                }
                else if(sectionMatch.Success)
                {
                    IniSection section = new IniSection(sectionMatch.Groups["Name"].Value);
                    currentSection = section.Parameters;
                    Sections.Add(section);
                }
                else if(!lineIsEmpty) //matches an empty line or comment line
                    throw new InvalidDataException("Input file contains incorrect formatting!");
            }
        }

        /// <summary>
        /// Gets or sets the setting with name key in the specified section.
        /// If it does not exist setting it will create it and getting it will return null.
        /// </summary>
        /// <param name="sectionName">name of section to place key in.</param>
        /// <param name="key">the unique key name linked to this setting</param>
        /// <returns>value associated with key or null if it does not exist</returns>
        public string this[string sectionName, string key]
        {
            get
            {
                if (Sections.ContainsKey(sectionName) &&
                    Sections[sectionName].Parameters.ContainsKey(key))
                    return Sections[sectionName].Parameters[key];
                else
                    return null;
            }
            set
            {
                if (Sections.ContainsKey(sectionName))
                {
                    IniParameters p = Sections[sectionName].Parameters;
                    if (p.ContainsKey(key))
                        p[key] = value;
                    else
                        p.Add(key, value);
                }
                else
                {
                    IniSection section = new IniSection(sectionName);
                    section.Parameters.Add(key, value);
                    Sections.Add(section);
                }
            }
        }

        /// <summary>
        /// Gets or sets the setting with name key in the INI files uncategorized paramaters.
        /// </summary>
        /// <param name="key">the unique key name linked to this setting</param>
        /// <returns></returns>
        public string this[string key]
        {
            get
            {
                if (Parameters.ContainsKey(key))
                    return Parameters[key];
                else
                    return null;
            }
            set
            {
                if (Parameters.ContainsKey(key))
                    Parameters[key] = value;
                else
                    Parameters.Add(key, value);
            }
        }

        /// <summary>
        /// Escapes a string so it is a legal value inside a .INI file.
        /// </summary>
        /// <param name="value">The string to escape</param>
        /// <returns>Escaped String</returns>
        private static string Escape(string value)
        {
            string pattern = @"[#;:\\=\t\r\n]";
            MatchEvaluator eval = (MatchEvaluator)delegate(Match m)
            {
                string s = m.Value;
                if(s == "\r")
                    return "\\r";
                else if(s == "\n")
                    return "\\n";
                else if(s == "\t")
                    return "\\t";
                else
                    return "\\" + s;
            };

            return Regex.Replace(value, pattern, eval);
            
        }

        /// <summary>
        /// Unescapes a string that was escaped inside a .INI file
        /// </summary>
        /// <param name="value">The string to Unescape</param>
        /// <returns>Unescaped String</returns>
        private static string Unescape(string value)
        {
            string pattern = @"\\([#;:\\=trn])";
            MatchEvaluator eval = (MatchEvaluator)delegate(Match m)
            {
                string s = m.Groups[1].Value;
                if(s == "r")
                    return "\r";
                else if(s == "n")
                    return "\n";
                else if(s == "t")
                    return "\t";
                else
                    return s;
            };

            return Regex.Replace(value, pattern, eval);
            
        }
        
    }


 //-------------Classes that are used with the INI file--------------//
    
    //just a section with a name and parameters
    public class IniSection
    {
        public IniParameters Parameters { get; set; }
        public string SectionName { get; set; }

        public IniSection(string sectionName)
        {
            SectionName = sectionName;
            Parameters = new IniParameters();
        }
    }

    //customized dictionary with uppercase key
    public class IniSections : Dictionary<string, IniSection>
    {
        public void Add(IniSection section)
        {
            base.Add(section.SectionName.ToUpper(), section);
        }

        new public void Add(string name, IniSection section)
        {
            if (section.SectionName.Equals(name, StringComparison.CurrentCultureIgnoreCase))
                throw new ArgumentOutOfRangeException("Name must be same as section name");
            else
                Add(section);
        }

        new public bool ContainsKey(string key)
        {
            return base.ContainsKey(key.ToUpper());
        }

        new public IniSection this[string key]
        {

            get
            {
                return base[key.ToUpper()];
            }
            set
            {
                base[key.ToUpper()] = value;
            }
        }
    }

    //basically a string dictionary with uppercase keys
    public class IniParameters : Dictionary<string, string>
    {
        new public void Add(string key, string value)
        {
            base.Add(key.ToUpper(), value);
        }

        new public bool ContainsKey(string key)
        {
            return base.ContainsKey(key.ToUpper());
        }

        new public string this[string key]
        {
            
            get
            {
                return base[key.ToUpper()];
            }
            set
            {
                base[key.ToUpper()] = value;
            }
        }
    }
}
